#include <iostream>

#include "interpreter.hpp"

#include <bython/codegen/llvm.hpp>
#include <bython/parser/grammar.hpp>
#include <lexy/action/parse.hpp>
#include <lexy_ext/report_error.hpp>
#include <lexy_ext/shell.hpp>
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

struct bython_prompt
{
  using encoding = lexy::default_encoding;
  using char_type = typename encoding::char_type;
};

auto interpreter::begin() -> void
{
  auto shell = lexy_ext::shell<bython_prompt> {};
  while (shell.is_open()) {
    auto input = shell.prompt_for_input();

    if (auto parsed = lexy::parse<grammar::inner_stmt>(input, lexy_ext::report_error);
        parsed.has_value())
    {
      auto ast = std::move(parsed).value();
      auto codegen = codegen::compile(std::make_unique<ast::mod>(std::move(ast)));

      // TODO: Live execution
    }
  }
}

}  // namespace bython::codegen