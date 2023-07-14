#pragma once

#include <memory>
#include <numeric>

#include <bython/ast/bases.hpp>
#include <bython/visitation/visitor.hpp>

namespace bython::matching
{

struct matcher
{
  virtual ~matcher() = default;

  virtual auto matches(ast::node const& ast) const -> bool = 0;
};

auto matches(ast::node const& ast, matcher const& matcher) -> bool;

}  // namespace bython::matching