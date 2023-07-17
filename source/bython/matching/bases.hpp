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

template<typename T, typename... Args>
auto lift(Args&&... args) -> std::unique_ptr<matcher>
{
  return std::move(std::make_unique<T>(std::forward<Args>(args)...));
}

}  // namespace bython::matching