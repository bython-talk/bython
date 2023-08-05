#include <iostream>
#include <string>

#include "jit.hpp"

#include <bython/codegen/builtin.hpp>
#include <bython/codegen/llvm.hpp>
#include <bython/parser/grammar.hpp>
#include <bython/parser/top_level_grammar.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/input/file.hpp>
#include <lexy_ext/report_error.hpp>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetParser.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

namespace bython::executor
{
struct jit_compiler::jit_compiler_pimpl
{
  /* LLVM details go here, e.g. llvm::ExecutionEngine, llvm::ExecutionSession
   * This helps us keep LLVM linkage private between bython_lib and consumers thereof,
   * for example, the bython tests
   */

  jit_compiler_pimpl()
  {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
  }

  ~jit_compiler_pimpl() = default;

  auto execute(std::filesystem::path const& input_file) -> int
  {
    auto file = lexy::read_file<lexy::utf8_encoding>(input_file.c_str());
    if (!file) {
      std::cerr << "Unable to read from " << input_file << "; check that it exists!";
      return -1;
    }

    auto contents = file.buffer();
    auto parsed = lexy::parse<grammar::top_level<grammar::mod>>(
        contents, lexy_ext::report_error.path(input_file.c_str()));
    if (parsed.is_error()) {
      std::cerr << parsed.errors() << "\n";
      return -1;
    }

    auto module_ = std::make_unique<ast::mod>(std::move(parsed).value());
    auto context = std::make_unique<llvm::LLVMContext>();

    auto codegen =
        codegen::compile(std::string {input_file.filename()}, std::move(module_), *context);
    codegen->setSourceFileName(std::string {input_file});
    codegen->setTargetTriple("x86_64-pc-linux-gnu");
    codegen->print(llvm::errs(), nullptr);

    std::string error;
    auto engine_builder = llvm::EngineBuilder(std::move(codegen));
    engine_builder.setTargetOptions(llvm::TargetOptions {});
    engine_builder.setErrorStr(&error);
    engine_builder.setEngineKind(llvm::EngineKind::JIT);
    engine_builder.setVerifyModules(true);
    engine_builder.setOptLevel(llvm::CodeGenOpt::None);

    auto engine = std::unique_ptr<llvm::ExecutionEngine>(engine_builder.create());

    if (!engine || error.size()) {
      std::cerr << "JIT Error: " << error << "\n";
      return -1;
    }

    for (auto&& builtin : {codegen::builtin_tag::put_i64, codegen::builtin_tag::putln_i64}) {
      auto bmetadata = codegen::builtin(*context, builtin);
      engine->addGlobalMapping(bmetadata.name, bmetadata.procedure_addr);
    }

    engine->finalizeObject();
    if (auto main_function = engine->FindFunctionNamed("main"); main_function != nullptr) {
      engine->runFunction(main_function, {});
      return 0;
    }

    std::cerr << "Cannot find main function! Exiting...\n";
    return -1;
  }
};

jit_compiler::jit_compiler()
    : impl {std::make_unique<jit_compiler::jit_compiler_pimpl>()}
{
}
jit_compiler::~jit_compiler() = default;

jit_compiler::jit_compiler(jit_compiler&&) = default;
auto jit_compiler::operator=(jit_compiler&&) noexcept -> jit_compiler& = default;

auto jit_compiler::execute(std::filesystem::path const& input_file) -> int
{
  return this->impl->execute(input_file);
}
}  // namespace bython::executor