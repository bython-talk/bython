#include <iostream>

#include "executors.hpp"

#include <bython/parser/entrypoints.hpp>

#include "llvm.hpp"

namespace bython::codegen
{

struct interpreter::interpreter_pimpl
{
  /* LLVM details go here, e.g. llvm::ExecutionEngine, llvm::ExecutionSession
   * This helps us keep LLVM linkage private between bython_lib and consumers thereof,
   * for example, the bython tests
   */
};

interpreter::interpreter()
    : impl {std::make_unique<interpreter::interpreter_pimpl>()}
{
}

interpreter::~interpreter() = default;

interpreter::interpreter(interpreter&&) = default;
auto interpreter::operator=(interpreter&&) noexcept -> interpreter& = default;

auto interpreter::repl(std::string_view code) -> void
{
  auto parsed = parser::module(code, lexy_ext::report_error);
  if (parsed.is_error()) {
    std::cerr << parsed.errors() << "\n";
    return;
  }

  auto ast = std::move(parsed).value();
  auto codegen = codegen::compile(std::make_unique<ast::mod>(std::move(ast)));
}


auto interpreter::retrieve_int_state(std::string_view entity) -> int64_t
{
  return 0;
}

}  // namespace bython::codegen