#pragma once

#include <bython/ast/operators.hpp>

#include "bases.hpp"

namespace bython::matching
{

struct unary_operator : matcher
{
  explicit unary_operator(ast::unop_tag op_)
      : op {op_}
  {
  }

  auto matches(ast::node const& ast) const -> bool override;

  ast::unop_tag op;
};

struct binary_operator : matcher
{
  explicit binary_operator(ast::binop_tag op_)
      : op {op_}
  {
  }

  auto matches(ast::node const& ast) const -> bool override;

  ast::binop_tag op;
};

struct comparison_operator : matcher
{
  explicit comparison_operator(ast::comparison_operator_tag op_)
      : op {op_}
  {
  }

  auto matches(ast::node const& ast) const -> bool override;

  ast::comparison_operator_tag op;
};

}  // namespace bython::matching