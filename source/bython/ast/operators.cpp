#include "operators.hpp"

namespace bython::ast
{
unary_operator::unary_operator(unop_tag op_)
    : op {op_}
{
}

auto unary_operator::tag() const -> ast::tag
{
  return ast::tag {tag::unary_operator};
}

auto binary_operator::tag() const -> ast::tag
{
  return ast::tag {tag::binary_operator};
}

auto comparison_operator::tag() const -> ast::tag
{
  return ast::tag {tag::comparison_operator};
}
}  // namespace bython::ast