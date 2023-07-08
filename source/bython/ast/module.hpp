#pragma once

#include <vector>

#include "bases.hpp"
#include "statement.hpp"

namespace bython::ast
{
struct mod : node
{
  explicit mod(statements body_)
      : body {std::move(body_)}
  {
  }

  ast::statements body;
};
};  // namespace bython::ast