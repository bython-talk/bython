#include "operators.hpp"

#include <bython/visitation/visitor.hpp>

namespace bython::ast
{
auto binary_operator::accept(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

auto unary_operator::accept(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

auto comparison_operator::accept(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}
}  // namespace bython::ast