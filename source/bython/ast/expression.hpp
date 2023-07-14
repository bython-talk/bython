#pragma once

#include <memory>
#include <string>
#include <vector>

#include "bases.hpp"

namespace bython::ast
{
struct expression : node
{
};

using expressions = std::vector<std::unique_ptr<expression>>;

enum class unary_operator
{
  plus,
  minus,
  bitnegate,
};

struct unary_operation final : expression
{
  unary_operation(unary_operator op_, std::unique_ptr<expression> rhs_);

  auto visit(visitation::visitor& visitor) const -> void override;

  unary_operator op;
  std::unique_ptr<expression> rhs;
};

enum class binary_operator
{
  // 2
  pow,

  // 3
  multiply,
  divide,
  modulo,

  // 4
  plus,
  minus,

  // 5
  bitshift_right_,
  bitshift_left_,

  // 8
  bitand_,

  // 9
  bitxor_,

  // 10
  bitor_,

  // 11
  booland,

  // 12
  boolor,
};

struct binary_operation final : expression
{
  // Ordered by C operator precedence
  binary_operator op;

  binary_operation(std::unique_ptr<expression> lhs_,
                   binary_operator binop_,
                   std::unique_ptr<expression> rhs_);

  auto visit(visitation::visitor& visitor) const -> void override;

  std::unique_ptr<expression> lhs, rhs;
};

enum class comparison_operator
{
  // 6
  lsr,
  leq,
  geq,
  grt,

  // 7
  eq,
  neq,
};

struct comparison final : expression
{
  comparison(ast::expressions operands_, std::vector<comparison_operator> ops_);

  auto visit(visitation::visitor& visitor) const -> void override;

  ast::expressions operands;
  std::vector<comparison_operator> ops;
};

struct variable final : expression
{
  explicit variable(std::string identifier_);

  auto visit(visitation::visitor& visitor) const -> void override;

  std::string identifier;
};

struct call final : expression
{
  call(std::string callee_,
       std::vector<std::unique_ptr<expression>> arguments_);

  auto visit(visitation::visitor& visitor) const -> void override;

  std::string callee;
  ast::expressions arguments;
};

struct integer final : expression
{
  explicit integer(int64_t value_);

  auto visit(visitation::visitor& visitor) const -> void override;

  int64_t value;
};

}  // namespace bython::ast
