#include "expression.hpp"

#include <bython/ast/expression.hpp>

#include "bases.hpp"

namespace bython::matching
{
auto unary_operation::matches(ast::node const& ast) const -> bool
{
  if (auto const* unary_op = ast::dyn_cast<ast::unary_operation>(&ast)) {
    return this->op == unary_op->op
        && matching::matches(*unary_op->rhs, *this->rhs_matcher);
  }
  return false;
}

auto binary_operation::matches(const ast::node& ast) const -> bool
{
  if (auto const* binary_op = ast::dyn_cast<ast::binary_operation>(&ast)) {
    return this->op == binary_op->op
        && matching::matches(*binary_op->lhs, *this->lhs_matcher)
        && matching::matches(*binary_op->rhs, *this->rhs_matcher);
  }
  return false;
}

auto comparison::chain(ast::comparison::compop op,
                       std::unique_ptr<matching::expression> against)
    -> comparison
{
  this->ops.emplace_back(op);
  this->operands.emplace_back(std::move(against));

  return std::move(*this);
}

auto comparison::matches(ast::node const& ast) const -> bool
{
  if (auto const* comps = ast::dyn_cast<ast::comparison>(&ast)) {
    if (this->ops != comps->ops
        || comps->operands.size() != this->operands.size())
    {
      return false;
    }

    for (decltype(comps->operands)::size_type i = 0; i < comps->operands.size();
         ++i)
    {
      if (!matching::matches(*comps->operands[i], *this->operands[i])) {
        return false;
      }
    }
    return true;
  }

  return false;
}

auto integer::matches(ast::node const& ast) const -> bool
{
  if (auto const* int_ = ast::dyn_cast<ast::integer>(&ast)) {
    return this->value == int_->value;
  }
  return false;
}

auto variable::matches(const ast::node &ast) const -> bool {
    if (auto const* var_ = ast::dyn_cast<ast::variable>(&ast)) {
        return this->identifier == var_->identifier;
    }

    return false;
}

}  // namespace bython::matching