#include "bases.hpp"

#include <bython/visitation/visitor.hpp>

namespace bython::matching
{

auto matches(std::unique_ptr<ast::node> const& ast,
             std::unique_ptr<matcher> const& matcher) -> bool
{
  return matches(*ast, *matcher);
}

auto matches(ast::node const& ast, matcher const& matcher) -> bool
{
  return matcher.matches(ast);
}
}  // namespace bython::matching