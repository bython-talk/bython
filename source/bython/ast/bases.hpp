#pragma once

#include <memory>

namespace bython::visitation
{
struct visitor;
struct transformer;
}  // namespace bython::visitation

namespace bython::ast
{
struct node
{
  virtual ~node() = default;

  virtual auto accept(visitation::visitor& visitor) const -> void = 0;
  /*virtual auto accept(visitation::transformer& transformer) const
      -> std::unique_ptr<node> = 0;*/
};

template<typename T>
auto isa(node const* ast) -> bool
{
  return dynamic_cast<T*>(ast) != nullptr;
}

template<typename T>
auto isa(node* ast) -> bool
{
  return dynamic_cast<T*>(ast) != nullptr;
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