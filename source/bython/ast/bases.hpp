#pragma once

#include <memory>

#include <boost/uuid/uuid.hpp>

#include "tags.hpp"

namespace bython::ast
{

struct node
{
  node();

  virtual ~node() = default;

  virtual auto tag() const -> ast::tag = 0;

  boost::uuids::uuid uuid;
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