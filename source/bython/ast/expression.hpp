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

struct unary_operation : expression
{
  enum class unop
  {
    plus,
    minus,
    bitnegate,
  } op;

  unary_operation(unop op_, std::unique_ptr<expression> rhs_)
      : op {op_}
      , rhs {std::move(rhs_)}
  {
  }

  std::unique_ptr<expression> rhs;
};

struct binary_operation : expression
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

    // 6
    lsr,
    leq,
    geq,
    grt,

    // 7
    eq,
    neq,

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
                   std::unique_ptr<expression> rhs_)
      : expression {}
      , op(binop_)
      , lhs {std::move(lhs_)}
      , rhs {std::move(rhs_)}
  {
  }

  std::unique_ptr<expression> lhs, rhs;
};

struct variable : expression
{
  explicit variable(std::string identifier_)
      : identifier {std::move(identifier_)}
  {
  }

  std::string identifier;
};

struct call : expression
{
  call(std::string callee_, std::vector<std::unique_ptr<expression>> arguments_)
      : callee {std::move(callee_)}
      , arguments {std::move(arguments_)}
  {
  }

  std::string callee;
  expressions arguments;
};

}  // namespace bython::ast
