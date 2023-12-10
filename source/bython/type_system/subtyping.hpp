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
  uint_to_single,
  uint_to_double,
  sint_to_single,
  sint_to_double,
  single_to_double,
  boolify,
};

auto try_subtype_impl(type const& tau, type const& alpha) -> std::optional<subtyping_rule>;
}  // namespace bython::type_system