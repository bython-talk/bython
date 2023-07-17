#include "operators.hpp"

#include <bython/ast/operators.hpp>

namespace bython::matching
{
auto binary_operator::matches(const ast::node& ast) const -> bool
{
  if (auto const* binary_op = ast::dyn_cast<ast::binary_operator>(&ast)) {
    return binary_op->op == this->op;
  }
  return false;
}

auto unary_operator::matches(const ast::node& ast) const -> bool
{
  if (auto const* unary = ast::dyn_cast<ast::unary_operator>(&ast)) {
    return unary->op == this->op;
  }
  return false;
}

auto comparison_operator::matches(const ast::node& ast) const -> bool
{
  if (auto const* comparison_op = ast::dyn_cast<ast::comparison_operator>(&ast))
  {
    return comparison_op->op == this->op;
  }
  return false;
}

}  // namespace bython::matching