#pragma once

#include <memory>

#include "tags.hpp"

namespace bython::ast
{

struct node
{
  virtual ~node() = default;

  virtual auto tag() const -> ast::tag = 0;
};

template<typename T>
auto isa(node const* ast) -> bool
{
  return dynamic_cast<T*>(ast) != nullptr;
}

template<typename T>
auto isa(node const& ast) -> bool
{
  return dynamic_cast<T*>(&ast) != nullptr;
}

template<typename T>
auto dyn_cast(node const* ast) -> T const*
{
  return dynamic_cast<T const*>(ast);
}

template<typename T>
auto dyn_cast(node* ast) -> T*
{
  return dynamic_cast<T*>(ast);
}

template<typename T>
auto dyn_cast(node const& ast) -> T const*
{
  return dynamic_cast<T const*>(&ast);
}

template<typename T>
auto dyn_cast(node& ast) -> T*
{
  return dynamic_cast<T*>(&ast);
}

}  // namespace bython::ast