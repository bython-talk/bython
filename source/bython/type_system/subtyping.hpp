#pragma once

#include <memory>
#include <optional>

#include "builtin.hpp"

namespace bython::type_system
{

enum class subtyping_rule
{
  identity,
  uint_promotion,
  sint_promotion,
};

auto try_subtype_impl(type const& tau, type const& alpha) -> std::optional<subtyping_rule>;
}  // namespace bython::type_system