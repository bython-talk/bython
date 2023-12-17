#pragma once

#include <cinttypes>
#include <compare>
#include <type_traits>

namespace bython::ast
{

struct tag
{
private:
  enum class ranges : std::uint32_t
  {
    expression = 0x000,
    statement = 0x100,
    misc = 0x200,
  };

public:
  enum expression : std::uint32_t
  {
    unary_operation = std::underlying_type_t<ranges>(ranges::expression) + 1,
    binary_operation,
    comparison,
    variable,
    call,
    signed_integer,
    unsigned_integer,
  };

  enum statement : std::uint32_t
  {
    type_definition = std::underlying_type_t<ranges>(ranges::statement) + 1,
    let_assignment,
    expression_statement,

    for_,
    while_,
    conditional_branch,
    unconditional_branch,
    function_def,
    return_
  };

  enum misc : std::uint32_t
  {
    unary_operator = std::underlying_type_t<ranges>(ranges::misc) + 1,
    binary_operator,
    comparison_operator,
    mod,
    argument_list,
    parameter,
    parameter_list
  };

  tag(tag::expression expression_tag)
      : tag_ {expression_tag}
  {
  }

  tag(tag::statement statement_tag)
      : tag_ {statement_tag}
  {
  }

  tag(tag::misc statement_tag)
      : tag_ {statement_tag}
  {
  }

  auto is_expression() const -> bool;
  auto is_statement() const -> bool;
  auto is_misc() const -> bool;

  auto unwrap() const -> std::uint32_t;

private:
  std::uint32_t tag_;
};

}  // namespace bython::ast