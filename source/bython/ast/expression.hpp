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

struct unary_operation final : expression
{
  enum class unop
  {
    plus,
    minus,
    bitnegate,
  } op;

  unary_operation(unop op_, std::unique_ptr<expression> rhs_);

  auto visit(visitation::visitor& visitor) const -> void override;

  std::unique_ptr<expression> rhs;
};

struct binary_operation final : expression
{
  // Ordered by C operator precedence
  enum class binop
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

  } op;

  binary_operation(std::unique_ptr<expression> lhs_,
                   binop binop_,
                   std::unique_ptr<expression> rhs_);

  auto visit(visitation::visitor& visitor) const -> void override;

  std::unique_ptr<expression> lhs, rhs;
};

struct comparison final : expression
{
  enum class compop
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

  comparison(ast::expressions operands_, std::vector<compop> ops_);

  auto visit(visitation::visitor& visitor) const -> void override;

  ast::expressions operands;
  std::vector<compop> ops;
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
