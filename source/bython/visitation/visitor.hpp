#pragma once

#include <type_traits>

namespace bython::visitation
{

template<typename Derived, typename RetTy>
struct visitor
{
  using derived = Derived;
  using ret_ty = RetTy;

  virtual ~visitor() = default;

  virtual auto visit(derived const&) const -> ret_ty = 0;
};

template<typename Derived>
struct visitable
{
  virtual ~visitable() = default;

  using derived = Derived;

  template<typename Visitor>
  auto accept(Visitor& visitor) const
  {
    // Purposely don't specify return type and let
    // auto do it's work to deduce template usages
    auto const* downcast = static_cast<derived const*>(*this);
    return visitor.visit(*downcast);
  }
};
}  // namespace bython::visitation