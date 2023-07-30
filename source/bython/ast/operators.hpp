#pragma once

#include "bases.hpp"

namespace bython::ast
{

enum class unop_tag
{
  plus,
  minus,
  bitnegate,
};

struct unary_operator final : node
{
  explicit unary_operator(unop_tag op_)
      : op {op_}
  {
  }

  unop_tag op;
};

enum class binop_tag
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

struct binary_operator final : node
{
  explicit binary_operator(binop_tag op_)
      : op {op_}
  {
  }

  binop_tag op;
};

enum class comparison_operator_tag
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

struct comparison_operator final : node
{
  explicit comparison_operator(comparison_operator_tag op_)
      : op {op_}
  {
  }

  comparison_operator_tag op;
};

}  // namespace bython::ast