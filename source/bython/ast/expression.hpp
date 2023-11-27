#pragma once

#include <memory>
#include <string>
#include <vector>

#include "bases.hpp"
#include "operators.hpp"

namespace bython::ast
{
struct expression : node
{
};

using expression_ptr = std::unique_ptr<expression>;
using expressions = std::vector<expression_ptr>;

struct unary_operation final : expression
{
  unary_operation(unop_tag op_, std::unique_ptr<expression> rhs_);

  unary_operator op;
  std::unique_ptr<expression> rhs;

  auto tag() const -> ast::tag;
};

struct binary_operation final : expression
{
  binary_operation(std::unique_ptr<expression> lhs_,
                   binop_tag binop_,
                   std::unique_ptr<expression> rhs_);

  std::unique_ptr<expression> lhs;
  binary_operator op;
  std::unique_ptr<expression> rhs;

  auto tag() const -> ast::tag;
};

struct comparison final : expression
{
  comparison(ast::expressions operands_, std::vector<comparison_operator_tag> ops_);

  auto add_operator(bython::ast::comparison_operator_tag op) -> void;
  auto add_operand(std::unique_ptr<expression> expr) -> void;

  ast::expressions operands;
  std::vector<std::unique_ptr<node>> ops;

  auto tag() const -> ast::tag;
};

struct variable final : expression
{
  explicit variable(std::string identifier_);

  std::string identifier;

  auto tag() const -> ast::tag;
};

struct argument_list final : node
{
  explicit argument_list(ast::expressions arguments_);
  ast::expressions arguments;

  auto tag() const -> ast::tag;
};

struct call final : expression
{
  call(std::string callee_, argument_list arguments_);

  std::string callee;
  argument_list arguments;

  auto tag() const -> ast::tag;
};

struct integer final : expression
{
  explicit integer(int64_t value_);

  int64_t value;

  auto tag() const -> ast::tag;
};

}  // namespace bython::ast
