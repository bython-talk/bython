#pragma once

#include <memory>

#include <bython/visitation/visitor.hpp>

namespace bython::ast
{

struct node : visitation::visitable<node>
{
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

}  // namespace bython::ast