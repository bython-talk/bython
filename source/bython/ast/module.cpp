#include "module.hpp"

#include <bython/visitation.hpp>

namespace bython::ast
{
mod::mod(statements body_)
    : body {std::move(body_)}
{
}

auto mod::visit(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}
}  // namespace bython::ast