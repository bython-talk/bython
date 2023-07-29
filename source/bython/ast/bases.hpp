#pragma once

#include <memory>

namespace bython::ast
{

struct node
{
  virtual ~node() = default;
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