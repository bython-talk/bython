#include <array>
#include <memory>
#include <optional>

#include "subtyping.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace
{
namespace ts = bython::type_system;

struct subtype_rule
{
  virtual ~subtype_rule() = default;
  virtual auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtype_converter> = 0;
};

/**
 * \tau <: \tau
 */
struct identity_rule final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtype_converter>
  {
    if (tau != alpha) {
      return std::nullopt;
    }

    return [](llvm::IRBuilder<>& /*builder*/, llvm::Value* expr, llvm::Type* /*dest*/)
    { return expr; };
  }
} const identity;

/**
 * \eUInt <: \eBiggerUInt
 */
struct unsigned_integer_rule final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtype_converter>
  {
    if (tau.tag() != ts::type_tag::uint || alpha.tag() != ts::type_tag::uint) {
      return std::nullopt;
    }

    auto const& tau_uint = dynamic_cast<ts::uint const&>(tau);
    auto const& alpha_uint = dynamic_cast<ts::uint const&>(alpha);

    if (tau_uint.width > alpha_uint.width) {
      return std::nullopt;
    }

    return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest) -> llvm::Value*
    { return builder.CreateIntCast(expr, dest, /*isSigned=*/false, "uint.prom"); };
  }
} const unsigned_integer;

/**
 * \eSInt <: \eBiggerSInt
 */
struct signed_integer_rule final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtype_converter>
  {
    if (tau.tag() != ts::type_tag::sint || alpha.tag() != ts::type_tag::sint) {
      return std::nullopt;
    }

    auto const& tau_sint = dynamic_cast<ts::sint const&>(tau);
    auto const& alpha_sint = dynamic_cast<ts::sint const&>(alpha);

    if (tau_sint.width > alpha_sint.width) {
      return std::nullopt;
    }

    return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest) -> llvm::Value*
    { return builder.CreateIntCast(expr, dest, /*isSigned=*/true, "sint.prom"); };
  }
} const signed_integer;

/*
struct single2double final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtype_converter>
  {
    if (tau.tag() == ts::type_tag::single_fp || alpha.tag() == ts::type_tag::double_fp) {
      return std::nullopt;
    }

    return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest)
    { return builder.CreateFPExt(expr, dest, "fp.ext"); };
  }
} const floating_point;

struct integer_to_float_rule final : subtype_rule
{
  auto try_subtype(ts::type const& tau, ts::type const& alpha) const
      -> std::optional<ts::subtype_converter>
  {
    auto* tau_int = dynamic_cast<ts::uint*>(tau.type.definition(context));
    auto* alpha_float = dynamic_cast<ts::>(tau.type.definition(context));

    if (tau.type->isIntegerTy() && alpha.type->isFloatingPointTy()) {
      return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest) {

      }
    }

    return std::nullopt;
  }
} const int2float;
*/

}  // namespace

namespace bython::type_system
{
static auto const rules =
    std::array<subtype_rule const*, 3> {{&identity, &unsigned_integer, &signed_integer}};

auto try_subtype_impl(ts::type const& tau, ts::type const& alpha) -> std::optional<ts::subtype_converter>
{
  for (auto&& rule : rules) {
    if (auto converter = rule->try_subtype(tau, alpha)) {
      return converter;
    }
  }

  return std::nullopt;
}
}  // namespace bython::type_system