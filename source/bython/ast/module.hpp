#pragma once

#include <vector>

#include "bases.hpp"
#include "statement.hpp"

namespace bython::ast
{
struct mod final : node
{
  explicit mod(statements body_);

  ast::statements body;

  auto tag() const -> ast::tag;
};
}  // namespace bython::ast