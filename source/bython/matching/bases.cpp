#include "bases.hpp"

#include <bython/visitation/visitor.hpp>

namespace bython::matching
{

auto matches(ast::node const& ast, matcher const& matcher) -> bool
{
  return matcher.matches(ast);
}
}  // namespace bython::matching