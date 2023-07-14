#pragma once

#include <vector>

#include "bases.hpp"
#include "statement.hpp"

namespace bython::ast
{
struct mod final : node
{
  explicit mod(statements body_);

  auto visit(visitation::visitor& visitor) const -> void override;

  ast::statements body;
};
}  // namespace bython::ast