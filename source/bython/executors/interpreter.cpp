#include <iostream>

#include "interpreter.hpp"

#include <bython/codegen/builtin.hpp>
#include <bython/codegen/llvm.hpp>
#include <bython/parser/grammar.hpp>
#include <bython/parser/top_level_grammar.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/action/scan.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/report_error.hpp>
#include <lexy_ext/shell.hpp>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/TargetSelect.h>

namespace bython::executor
{
using namespace std::literals::string_literals;

struct bython_prompt
{
  using encoding = lexy::default_encoding;
  using char_type = typename encoding::char_type;
};

// https://github.com/foonathan/lexy/blob/main/examples/shell.cpp

namespace shell
{
enum class directive
{
  codegen,
  dump,
  exit
};
}  // namespace shell

struct directive
{
  struct unknown_directive
  {
    static constexpr auto name = "unknown directive";
  };

  // Map pre-defined directives.
  static constexpr auto directives = lexy::symbol_table<shell::directive>  //
                                         .map<LEXY_SYMBOL("codegen")>(shell::directive::codegen)  //
                                         .map<LEXY_SYMBOL("dump")>(shell::directive::dump)  //
                                         .map<LEXY_SYMBOL("exit")>(shell::directive::exit)  //
      ;

  static constexpr auto rule = []
  {
    auto pattern = lexy::dsl::identifier(lexy::dsl::ascii::alpha);
    auto directive = lexy::dsl::symbol<directives>(pattern).error<unknown_directive>;

    // A directive is optional, but it also consumes the argument separator if parsed.
    return lexy::dsl::colon >> directive;
  }();

  // Forward the existing directive but default to codegen.
  static constexpr auto value =
      lexy::bind(lexy::forward<shell::directive>, lexy::_1 or shell::directive::codegen);
};

struct interpreter::interpreter_pimpl
{
  /* LLVM details go here, e.g. llvm::ExecutionEngine, llvm::ExecutionSession
   * This helps us keep LLVM linkage private between bython_lib and consumers thereof,
   * for example, the bython tests
   */
  interpreter_pimpl() = delete;

  interpreter_pimpl(std::unique_ptr<llvm::orc::ExecutionSession> session_,
                    llvm::orc::JITTargetMachineBuilder jtmb,
                    llvm::DataLayout data_layout_)
      : session {std::move(session_)}
      , data_layout {std::move(data_layout_)}
      , object_layer {*session, []() { return std::make_unique<llvm::SectionMemoryManager>(); }}
      , compile_layer {*session,
                       object_layer,
                       std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(jtmb))}
      , context {std::make_unique<llvm::LLVMContext>()}
      , main {session->createBareJITDylib("<repl>")}
  {
    auto builtin_symbols = llvm::orc::SymbolMap{};

    for (auto&& b : {codegen::builtin_tag::put_i64,
                     codegen::builtin_tag::putln_i64,
                     codegen::builtin_tag::put_f32})
    {
      auto metadata = codegen::builtin(*this->context.getContext(), b);

      auto name = this->session->intern(metadata.name);
      auto location =
          llvm::pointerToJITTargetAddress(std::bit_cast<void*>(metadata.procedure_addr));
      auto flags = llvm::JITSymbolFlags::Callable | llvm::JITSymbolFlags::Absolute;

      builtin_symbols[name] = {location, flags};
    }

    llvm::ExitOnError("Failed to insert builtin functions")(
        this->main.define(llvm::orc::absoluteSymbols(builtin_symbols)));
  }

  ~interpreter_pimpl()
  {
    if (auto error = this->session->endSession(); error) {
      llvm::errs() << error << "\n";
    }
  }

  using top_level_stmt = grammar::repl_top_level<grammar::outer_stmt>;
  using top_level_expr = grammar::repl_top_level<grammar::expression_statement>;

  auto activate() -> void
  {
    auto jit = llvm::ExitOnError("Failed to create LLJitBuilder")(
        llvm::orc::LLJITBuilder().setNumCompileThreads(4).create());

    auto shell = lexy_ext::shell</*bython_prompt*/> {};
    for (auto iter = 0UL; shell.is_open(); ++iter) {
      auto input = shell.prompt_for_input();

      std::string errors;
      auto error_writer = std::back_insert_iterator(errors);
      if (auto definition =
              lexy::parse<top_level_stmt>(input, lexy_ext::report_error.to(error_writer));
          definition.has_value())
      {
        auto module_name = (llvm::Twine {"repl"} + std::to_string(iter)).str();
        auto ast = std::unique_ptr<bython::ast::node>(std::move(definition).value());
        auto module_ = codegen::compile(module_name, std::move(ast), *this->context.getContext());
        llvm::outs() << *module_ << "\n";

        llvm::ExitOnError("Failed to add module:"s + module_name + "\n")(
            this->add_module(llvm::orc::ThreadSafeModule {std::move(module_), this->context}));
      }

      /*
      TODO: Currently disabled until we discover how to codegen calls for functions that are defined elsewhere
      else if (auto inner_stmt =
                   lexy::parse<top_level_expr>(input, lexy_ext::report_error.to(error_writer));
               inner_stmt.has_value())
      {
        auto module_name = (llvm::Twine {"__anon_expr"} + std::to_string(iter)).str();
        auto anon_expr = std::make_unique<llvm::Module>(module_name, *this->context.getContext());

        auto ast = std::unique_ptr<ast::statement>(std::move(inner_stmt).value());

        auto anon_function_body = ast::statements {};
        anon_function_body.emplace_back(std::move(ast));
        auto anon_function = std::make_unique<ast::function_def>(
            module_name, ast::parameters {}, std::move(anon_function_body));

        codegen::compile(std::move(anon_function), *anon_expr);
        llvm::outs() << *anon_expr << "\n";

        llvm::ExitOnError("Failed to add module:"s + module_name)(
            this->add_module(llvm::orc::ThreadSafeModule {std::move(anon_expr), this->context}));

        auto entrypoint_symbol = llvm::ExitOnError("Failed to find codegenned expr "s + module_name
                                                   + "\n")(this->lookup(module_name));
        auto entrypoint_addr = entrypoint_symbol.getAddress();
        auto entrypoint = std::bit_cast<void (*)()>(entrypoint_addr);
        entrypoint();
      }
      */

      else
      {
        llvm::errs() << errors << "\n\n";
      }
    }
  }

private:
  auto add_module(llvm::orc::ThreadSafeModule module) -> llvm::Error
  {
    return this->compile_layer.add(this->main, std::move(module));
  }

  auto lookup(llvm::StringRef symbol_name) -> llvm::Expected<llvm::JITEvaluatedSymbol>
  {
    return this->session->lookup({&this->main}, symbol_name);
  }

  std::unique_ptr<llvm::orc::ExecutionSession> session;
  llvm::DataLayout data_layout;

  llvm::orc::RTDyldObjectLinkingLayer object_layer;
  llvm::orc::IRCompileLayer compile_layer;

  llvm::orc::ThreadSafeContext context;
  llvm::orc::JITDylib& main;
};

interpreter::interpreter()
    // Delayed initialisation
    : impl {nullptr}
{
}

interpreter::~interpreter() = default;

interpreter::interpreter(interpreter&&) = default;
auto interpreter::operator=(interpreter&&) noexcept -> interpreter& = default;

auto interpreter::begin() -> void
{
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();

  auto ctrl = llvm::ExitOnError("Failed to create orc::SelfExecutorProcessControl")(
      llvm::orc::SelfExecutorProcessControl::Create());
  auto session = std::make_unique<llvm::orc::ExecutionSession>(std::move(ctrl));

  auto jtmb =
      llvm::orc::JITTargetMachineBuilder(session->getExecutorProcessControl().getTargetTriple());
  auto data_layout =
      llvm::ExitOnError("Failed to retrieve data layout")(jtmb.getDefaultDataLayoutForTarget());

  this->impl = std::make_unique<interpreter::interpreter_pimpl>(
      std::move(session), std::move(jtmb), std::move(data_layout));
  this->impl->activate();
}

}  // namespace bython::executor