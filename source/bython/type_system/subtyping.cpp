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
 * \eUInt <: \eBiggerUInt
 */
struct unsigned_integer_rule final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtyping_rule>
  {
    if (tau.tag() != ts::type_tag::uint || alpha.tag() != ts::type_tag::uint) {
      return std::nullopt;
    }

    auto const& tau_uint = dynamic_cast<ts::uint const&>(tau);
    auto const& alpha_uint = dynamic_cast<ts::uint const&>(alpha);

    if (tau_uint.width > alpha_uint.width) {
      return std::nullopt;
    }

    return ts::subtyping_rule::uint_promotion;
  }
} const unsigned_integer;

/**
 * \eSInt <: \eBiggerSInt
 */
struct signed_integer_rule final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtyping_rule>
  {
    if (tau.tag() != ts::type_tag::sint || alpha.tag() != ts::type_tag::sint) {
      return std::nullopt;
    }

    auto const& tau_sint = dynamic_cast<ts::sint const&>(tau);
    auto const& alpha_sint = dynamic_cast<ts::sint const&>(alpha);

    if (tau_sint.width > alpha_sint.width) {
      return std::nullopt;
    }

    return ts::subtyping_rule::sint_promotion;
  }
} const signed_integer;

/*
struct single2double final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtyping_rule>
  {
    if (tau.tag() == ts::type_tag::single_fp || alpha.tag() == ts::type_tag::double_fp) {
      return std::nullopt;
    }

    return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest)
    { return builder.CreateFPExt(expr, dest, "fp.ext"); };
  }
} const floating_point;
*/

/**
 * \e{S,U}Int <: \eF{32,64}
 */
struct integer_to_fp_rule final : subtype_rule
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
} const int2float;

}  // namespace

namespace bython::type_system
{
static auto const rules = std::array<subtype_rule const*, 4> {
    {&identity, &unsigned_integer, &signed_integer, &int2float}};

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