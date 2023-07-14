#include "compound.hpp"

#include <bython/visitation.hpp>

namespace bython::ast
{
compound::compound(statements body_)
    : body {std::move(body_)}
{
}

auto for_::visit(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

auto while_::visit(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

auto function_def::visit(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

}  // namespace bython::ast