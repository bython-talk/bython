#include <iostream>
#include <string>

#include "jit.hpp"

#include <bython/codegen/llvm.hpp>
#include <bython/parser/grammar.hpp>
#include <bython/parser/top_level_grammar.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/input/file.hpp>  
#include <lexy_ext/report_error.hpp>
#include <llvm/ExecutionEngine/MCJIT.h>

namespace bython::executor
{
struct jit_compiler::jit_compiler_pimpl
{
  /* LLVM details go here, e.g. llvm::ExecutionEngine, llvm::ExecutionSession
   * This helps us keep LLVM linkage private between bython_lib and consumers thereof,
   * for example, the bython tests
   */
  auto execute(std::filesystem::path const& input_file) -> int
  {
    auto file = lexy::read_file<lexy::utf8_encoding>(input_file.c_str());
    if (!file) {
      std::cerr << "Unable to read from " << input_file << "; check that it exists!";
      return -1;
    }

    auto contents = file.buffer();
    auto parsed = lexy::parse<grammar::top_level<grammar::mod>>(contents, lexy_ext::report_error);
    if (parsed.is_error()) {
      std::cerr << parsed.errors() << "\n";
      return -1;
    }

    auto module_ = std::make_unique<ast::mod>(std::move(parsed).value());
    auto codegen = codegen::compile(std::move(module_));

    std::string error;
    auto engine = llvm::EngineBuilder(std::move(codegen))
        .setErrorStr(&error)
        .setEngineKind(llvm::EngineKind::JIT)
        .setVerifyModules(true)
        // .setOptLevel
        .create();
    if (!engine) {
        std::cerr << "JIT Error: " << error << "\n";
        return -1;
    }
    engine->finalizeObject();

    // Assume signature of function
    using main_signature = void(*)(void);

    auto main_function_addr = engine->getFunctionAddress("main");
    auto mainptr = reinterpret_cast<main_signature>(main_function_addr);

    mainptr();
    return 0;
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
}  // namespace bython::codegen