#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>

#include "environment.hpp"

#include "builtin.hpp"
#include "bython/ast.hpp"
#include "bython/ast/visitor.hpp"
#include "bython/type_system/subtyping.hpp"
#include "bython/type_system/inference.hpp"

namespace bython
{
using namespace bython::ast;
namespace ts = bython::type_system;

struct inference_visitor : visitor<inference_visitor, ts::type const&>
{
};

}  // namespace bython

namespace bython::type_system
{
auto environment::initialise_with_builtins() -> environment
{
  auto env = environment {};

  // boolean
  env.add("bool", std::make_unique<boolean>());

  // uint{8, 16, 32, 64}_t
  env.add("u8", std::make_unique<uint>(8));
  env.add("u16", std::make_unique<uint>(16));
  env.add("u32", std::make_unique<uint>(32));
  env.add("u64", std::make_unique<uint>(64));

  // int{8, 16, 32, 64}_t
  env.add("i8", std::make_unique<sint>(8));
  env.add("i16", std::make_unique<sint>(16));
  env.add("i32", std::make_unique<sint>(32));
  env.add("i64", std::make_unique<sint>(64));

  // f{32, 64}
  env.add("f32", std::make_unique<single_fp>());
  env.add("f64", std::make_unique<double_fp>());

  return env;
}

auto environment::add(std::string tname, std::unique_ptr<type> type) -> void
{
  auto&& inserted = this->m_visible_types.emplace_back(std::move(type));
  this->m_typename_to_typeptr.emplace(std::move(tname), inserted.get());
}

auto environment::lookup(std::string_view tname) const -> std::optional<type_system::type*>
{
  if (auto it = this->m_typename_to_typeptr.find(tname); it != this->m_typename_to_typeptr.end()) {
    return it->second;
  }
  return std::nullopt;
}

auto environment::try_subtype(type_system::type const& tau, type_system::type const& alpha) const
    -> std::optional<type_system::subtyping_rule>
{
  return try_subtype_impl(tau, alpha);
}

auto environment::infer(ast::expression const& expr) const
    -> std::optional<type_system::type*>
{
  return try_infer_impl(expr, *this);
}

}  // namespace bython::type_system