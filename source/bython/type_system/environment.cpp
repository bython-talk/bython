#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>

#include "environment.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include "builtin.hpp"
#include "bython/ast.hpp"
#include "bython/ast/visitor.hpp"
#include "bython/type_system/subtyping.hpp"

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

  env.add("u8", std::make_unique<uint>(8));
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
  this->m_id_to_ts.emplace(std::move(tname), inserted.get());
}

auto environment::lookup(std::string_view tname) const -> std::optional<type_system::type const*>
{
  if (auto it = this->m_id_to_ts.find(tname); it != this->m_id_to_ts.end()) {
    return it->second;
  }
  return std::nullopt;
}

auto environment::lookup(ast::expression const& expr) const
    -> std::optional<type_system::type const*>
{
  if (auto it = this->m_expr_to_ts.find(expr.uuid); it != this->m_expr_to_ts.end()) {
    return it->second;
  }
  return std::nullopt;
}

auto environment::typeify_expression(ast::expression const& expr, std::string_view tname)
    -> std::optional<type_system::type const*>
{
  if (auto type_reference = this->m_id_to_ts.find(tname); type_reference != this->m_id_to_ts.end())
  {
    this->m_expr_to_ts.emplace(expr.uuid, type_reference->second);
    return type_reference->second;
  }
  return std::nullopt;
}

auto environment::try_subtype(type_system::type const& tau, type_system::type const& alpha)
    -> std::optional<type_system::subtype_converter>
{
  return try_subtype_impl(tau, alpha);
}

auto environment::infer(ast::expression const& expr) const
    -> std::optional<type_system::type const*>
{
  return std::nullopt;
}

}  // namespace bython::type_system