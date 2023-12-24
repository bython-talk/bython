#pragma once

#include <vector>

#include "bases.hpp"
#include "expression.hpp"
#include "statement.hpp"

namespace bython::ast
{
struct mod final : node
{
  explicit mod(statements body_);

  ast::statements body;

  auto tag() const -> ast::tag;
};

struct expr_mod final : node
{
  explicit expr_mod(std::vector<std::unique_ptr<ast::expression>> body_);

  std::vector<std::unique_ptr<ast::expression>> body;

  auto tag() const -> ast::tag;
};

}  // namespace bython::ast