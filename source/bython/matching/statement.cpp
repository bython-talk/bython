#include "statement.hpp"

#include <bython/ast/statement.hpp>

#include "bases.hpp"

namespace bython::matching
{
auto assignment::matches(ast::node const& ast) const -> bool
{
  if (auto const* assgn = ast::dyn_cast<ast::assignment>(&ast)) {
    return this->lhs == assgn->lhs && matching::matches(*assgn->rhs, *this->rhs);
  }

  return false;
}
}  // namespace bython::matching