#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

#include "inference.hpp"

#include "bython/ast.hpp"
#include "bython/ast/expression.hpp"
#include "bython/ast/operators.hpp"
#include "bython/ast/statement.hpp"
#include "bython/ast/visitor.hpp"
#include "bython/type_system/builtin.hpp"
#include "bython/type_system/environment.hpp"

namespace
{

using namespace bython::ast;  // This line is required for the visitor macros to function properly
namespace ts = bython::type_system;

template<typename... Us>
struct cast
{
  template<typename... Ts>
  static auto multi(Ts&&... ts) -> std::tuple<Us&&...>
  {
    return std::forward_as_tuple(dynamic_cast<Us&&>(ts)...);
  }
};

struct inference_visitor : visitor<inference_visitor, std::optional<ts::type*>>
{
  explicit inference_visitor(ts::environment const& environment)
      : env {environment}
  {
  }

  BYTHON_VISITOR_IMPL(unary_operation, unop)
  {
    auto rhs_type = this->visit(*unop.rhs);
    return rhs_type;
    // if (!rhs_type) { return std::nullopt; }
  }

  BYTHON_VISITOR_IMPL(binary_operation, binop)
  {
    auto lhs_type = this->visit(*binop.lhs);
    if (!lhs_type) {
      return std::nullopt;
    }
    auto lhs_tag = lhs_type.value()->tag();

    std::optional<ts::type*> rhs_type = std::nullopt;
    std::optional<ts::type_tag> rhs_tag = std::nullopt;
    if (binop.op.op != binop_tag::as) {
      if (rhs_type = this->visit(*binop.rhs); !rhs_type) {
        return std::nullopt;
      }
      rhs_tag = rhs_type.value()->tag();
    }

    switch (binop.op.op) {
      case binop_tag::as: {
        if (auto target = dyn_cast<variable>(*binop.rhs)) {
          return this->env.lookup(target->identifier);
        }
        return std::nullopt;
      }

      case binop_tag::pow: {
        // TODO: Solve using parameter to function rule?
        break;
      }
      case binop_tag::multiply:
      case binop_tag::divide:
      case binop_tag::plus:
      case binop_tag::minus:
      case binop_tag::modulo: {
        // Both unsigned; go with larger type
        if (lhs_tag == ts::type_tag::uint && *rhs_tag == ts::type_tag::uint) {
          auto [lhs_uint, rhs_uint] =
              cast<ts::uint, ts::uint>::multi(*lhs_type.value(), *rhs_type.value());
          return std::addressof(lhs_uint.width >= rhs_uint.width ? lhs_uint : rhs_uint);
        }

        // Both signed; go with larger type
        if (lhs_tag == ts::type_tag::sint && *rhs_tag == ts::type_tag::sint) {
          auto [lhs_sint, rhs_sint] =
              cast<ts::sint, ts::sint>::multi(*lhs_type.value(), *rhs_type.value());
          return std::addressof(lhs_sint.width >= rhs_sint.width ? lhs_sint : rhs_sint);
        }

        // If both are the same size, then promote to the unsigned one, otherwise promote to the
        // larger one
        if (lhs_tag == ts::type_tag::uint && *rhs_tag == ts::type_tag::sint) {
          auto [lhs_uint, rhs_sint] =
              cast<ts::uint, ts::sint>::multi(*lhs_type.value(), *rhs_type.value());
          return lhs_uint.width >= rhs_sint.width ? static_cast<ts::type*>(&lhs_uint)
                                                  : static_cast<ts::type*>(&rhs_sint);
        }

        if (lhs_tag == ts::type_tag::sint && *rhs_tag == ts::type_tag::uint) {
          auto [lhs_sint, rhs_uint] =
              cast<ts::sint, ts::uint>::multi(*lhs_type.value(), *rhs_type.value());
          return rhs_uint.width >= lhs_sint.width ? static_cast<ts::type*>(&rhs_uint)
                                                  : static_cast<ts::type*>(&lhs_sint);
        }

        return std::nullopt;
      }

      case binop_tag::bitshift_right_:
      case binop_tag::bitshift_left_: {
        if (rhs_tag != ts::type_tag::uint) {
          return std::nullopt;
        }
        return lhs_type;
      }
      case binop_tag::bitand_:
      case binop_tag::bitxor_:
      case binop_tag::bitor_:
      case binop_tag::booland:
      case binop_tag::boolor:
        break;
    }

    return std::nullopt;
  }

  BYTHON_VISITOR_IMPL(unsigned_integer, instance)
  {
    if (std::numeric_limits<std::uint8_t>::lowest() <= instance.value
        && instance.value <= std::numeric_limits<std::uint8_t>::max())
    {
      return this->env.lookup("u8");
    }

    if (std::numeric_limits<std::uint16_t>::lowest() <= instance.value
        && instance.value <= std::numeric_limits<std::uint16_t>::max())
    {
      return this->env.lookup("u16");
    }

    if (std::numeric_limits<std::uint32_t>::lowest() <= instance.value
        && instance.value <= std::numeric_limits<std::uint32_t>::max())
    {
      return this->env.lookup("u32");
    }

    return this->env.lookup("u64");
  }

  BYTHON_VISITOR_IMPL(signed_integer, instance)
  {
    if (std::numeric_limits<std::int8_t>::lowest() <= instance.value
        && instance.value <= std::numeric_limits<std::int8_t>::max())
    {
      return this->env.lookup("i8");
    }

    if (std::numeric_limits<std::int16_t>::lowest() <= instance.value
        && instance.value <= std::numeric_limits<std::int16_t>::max())
    {
      return this->env.lookup("i16");
    }

    if (std::numeric_limits<std::int32_t>::lowest() <= instance.value
        && instance.value <= std::numeric_limits<std::int32_t>::max())
    {
      return this->env.lookup("i32");
    }

    return this->env.lookup("i64");
  }

  BYTHON_VISITOR_IMPL(node, instance)
  {
    throw std::runtime_error {"Cannot perform inference; Unknown AST Node: "
                              + std::to_string(instance.tag().unwrap())};
  }

  ts::environment const& env;
};
}  // namespace

namespace bython::type_system
{
auto try_infer_impl(ast::expression const& expr, type_system::environment const& environment)
    -> std::optional<type_system::type*>
{
  auto visitor = inference_visitor {environment};
  return visitor.visit(expr);
}
}  // namespace bython::type_system