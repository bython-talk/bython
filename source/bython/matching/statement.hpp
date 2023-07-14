#pragma once

#include <bython/ast/statement.hpp>

#include "bases.hpp"
#include "expression.hpp"

namespace bython::matching
{
struct statement : matcher
{
};

struct assignment final : matching::statement
{
  assignment(std::string lhs_,
             std::unique_ptr<matching::expression> rhs_)
      : lhs {std::move(lhs_)}
      , rhs {std::move(rhs_)}
  {
  }

  auto matches(ast::node const& ast) const -> bool override;

  std::string lhs;
  std::unique_ptr<matching::expression> rhs;
};
}  // namespace bython::matching