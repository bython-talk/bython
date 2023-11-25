#include <array>
#include <memory>
#include <optional>

#include "subtyping.hpp"

#include "bython/type_system/builtin.hpp"

namespace
{
namespace ts = bython::type_system;

struct subtype_rule
{
  virtual ~subtype_rule() = default;
  virtual auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtyping_rule> = 0;
};

/**
 * \tau <: \tau
 */
struct identity_rule final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtyping_rule>
  {
    if (tau != alpha) {
      return std::nullopt;
    }

    return ts::subtyping_rule::identity;
  }
} const identity;

/**
 * \eSInt <: \eBiggerSInt
 * \eUInt <: \eBiggerUInt
 */
struct integer_promotion_rule final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtyping_rule>
  {
    if (tau.tag() == ts::type_tag::sint && alpha.tag() == ts::type_tag::sint) {
      auto const& tau_sint = dynamic_cast<ts::sint const&>(tau);
      auto const& alpha_sint = dynamic_cast<ts::sint const&>(alpha);

      if (tau_sint.width < alpha_sint.width) {
        return ts::subtyping_rule::sint_promotion;
      }
    }

    else if (tau.tag() == ts::type_tag::uint && alpha.tag() == ts::type_tag::uint)
    {
      auto const& tau_uint = dynamic_cast<ts::uint const&>(tau);
      auto const& alpha_uint = dynamic_cast<ts::uint const&>(alpha);

      if (tau_uint.width < alpha_uint.width) {
        return ts::subtyping_rule::uint_promotion;
      }
    }

    return std::nullopt;
  }
} const integer_promotion;

/**
 * \e{S,U}Int <: \eF{32,64}
 */
struct integer_to_floating_point_rule final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtyping_rule>
  {
    auto taut = tau.tag();
    auto alphat = alpha.tag();

    if (taut == ts::type_tag::uint && alphat == ts::type_tag::single_fp) {
      return ts::subtyping_rule::uint_to_single;
    }
    if (taut == ts::type_tag::uint && alphat == ts::type_tag::double_fp) {
      return ts::subtyping_rule::uint_to_double;
    }
    if (taut == ts::type_tag::sint && alphat == ts::type_tag::single_fp) {
      return ts::subtyping_rule::sint_to_single;
    }
    if (taut == ts::type_tag::sint && alphat == ts::type_tag::double_fp) {
      return ts::subtyping_rule::sint_to_double;
    }

    return std::nullopt;
  }
} const integer2floating_point;

/**
 * \eF32 <: \eF64
 */
struct fp_promotion_rule final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtyping_rule>
  {
    auto taut = tau.tag();
    auto alphat = alpha.tag();

    if (taut == ts::type_tag::single_fp && alphat == ts::type_tag::double_fp) {
      return ts::subtyping_rule::single_to_double;
    }

    return std::nullopt;
  }
} const fp_promotion;

}  // namespace

namespace bython::type_system
{
static auto const rules = std::array<subtype_rule const*, 4> {
    {&identity, &integer_promotion, &fp_promotion, &integer2floating_point}};

auto try_subtype_impl(ts::type const& tau, ts::type const& alpha)
    -> std::optional<ts::subtyping_rule>
{
  for (auto&& rule : rules) {
    if (auto matching_rule = rule->try_subtype(tau, alpha); matching_rule) {
      return matching_rule;
    }
  }

  return std::nullopt;
}
}  // namespace bython::type_system