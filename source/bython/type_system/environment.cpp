#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "environment.hpp"

#include <boost/uuid/uuid_io.hpp>

#include "builtin.hpp"
#include "bython/ast.hpp"
#include "bython/ast/statement.hpp"
#include "bython/ast/visitor.hpp"
#include "bython/type_system/inference.hpp"
#include "bython/type_system/subtyping.hpp"

namespace bython::type_system
{
auto environment::initialise_with_builtins() -> environment
{
  auto env = environment {};

  /// Types
  // void / unit
  [[maybe_unused]] auto voidt_ = env.add_new_named_type("void", std::make_unique<void_>());

  // boolean
  [[maybe_unused]] auto bool_ = env.add_new_named_type("bool", std::make_unique<boolean>());

  // uint{8, 16, 32, 64}_t
  [[maybe_unused]] auto u8 = env.add_new_named_type("u8", std::make_unique<uint>(8));
  [[maybe_unused]] auto u16 = env.add_new_named_type("u16", std::make_unique<uint>(16));
  [[maybe_unused]] auto u32 = env.add_new_named_type("u32", std::make_unique<uint>(32));
  [[maybe_unused]] auto u64 = env.add_new_named_type("u64", std::make_unique<uint>(64));

  // int{8, 16, 32, 64}_t
  [[maybe_unused]] auto i8 = env.add_new_named_type("i8", std::make_unique<sint>(8));
  [[maybe_unused]] auto i16 = env.add_new_named_type("i16", std::make_unique<sint>(16));
  [[maybe_unused]] auto i32 = env.add_new_named_type("i32", std::make_unique<sint>(32));
  [[maybe_unused]] auto i64 = env.add_new_named_type("i64", std::make_unique<sint>(64));

  // f{32, 64}
  [[maybe_unused]] auto f32 = env.add_new_named_type("f32", std::make_unique<single_fp>());
  [[maybe_unused]] auto f64 = env.add_new_named_type("f64", std::make_unique<double_fp>());

  /// Functions
  auto put_i64_ft =
      env.add_unnamed_type(std::make_unique<function_signature>(std::vector {i64}, voidt_));
  env.add_new_symbol("put_i64", put_i64_ft);

  auto put_u64_ft =
      env.add_unnamed_type(std::make_unique<function_signature>(std::vector {u64}, voidt_));
  env.add_new_symbol("put_u64", put_u64_ft);

  auto put_f32_ft =
      env.add_unnamed_type(std::make_unique<function_signature>(std::vector {f32}, voidt_));
  env.add_new_symbol("put_f32", put_f32_ft);

  auto put_f64_ft =
      env.add_unnamed_type(std::make_unique<function_signature>(std::vector {f32}, voidt_));
  env.add_new_symbol("put_f64", put_f64_ft);

  return env;
}

auto environment::add_new_named_type(std::string tname, std::unique_ptr<type> type)
    -> type_system::type*
{
  auto&& [inserted, _] = this->m_visible_types.emplace(std::move(type));
  this->m_typename_to_typeptr.emplace(std::move(tname), inserted->get());
  return inserted->get();
}

auto environment::add_unnamed_type(std::unique_ptr<type> type) -> type_system::type*
{
  auto&& [inserted, _] = this->m_visible_types.emplace(std::move(type));
  return inserted->get();
}

auto environment::add_new_function_type(ast::signature const& signature)
    -> std::optional<type_system::function_signature*>
{
  auto parameters = std::vector<type_system::type*> {};
  type_system::type* rettype = nullptr;

  for (auto&& parameter : signature.parameters.parameters) {
    if (auto ptype = this->lookup_type(parameter.hint); ptype) {
      parameters.emplace_back(*ptype);
    } else {
      return std::nullopt;
    }
  }

  if (signature.rettype) {
    auto rettype_ts = this->lookup_type(*signature.rettype);
    if (!rettype_ts) {
      return std::nullopt;
    }
    rettype = *rettype_ts;
  } else {
    rettype = this->lookup_type("void").value();
  }

  auto added_ft = this->add_new_named_type(
      signature.name,  // TODO: This is likely unsound, reconsinder "uniqueness"
      std::make_unique<type_system::function_signature>(std::move(parameters), rettype));
  return std::make_optional(dynamic_cast<type_system::function_signature*>(added_ft));
}

auto environment::lookup_type(std::string_view tname) const -> std::optional<type_system::type*>
{
  if (auto it = this->m_typename_to_typeptr.find(tname); it != this->m_typename_to_typeptr.end()) {
    return it->second;
  }
  return std::nullopt;
}

auto environment::add_new_symbol(std::string sname, type_system::type* type) -> void
{
  this->m_symbol_to_ts.emplace(sname, type);
}

auto environment::lookup_symbol(std::string_view symbol_name) const
    -> std::optional<type_system::type*>
{
  if (auto it = this->m_symbol_to_ts.find(symbol_name); it != this->m_symbol_to_ts.end()) {
    return it->second;
  }
  return std::nullopt;
}

auto environment::try_subtype(type_system::type const& tau, type_system::type const& alpha) const
    -> std::optional<type_system::subtyping_rule>
{
  return try_subtype_impl(tau, alpha);
}

auto environment::get_type(ast::expression const& expr) const -> std::optional<type_system::type*>
{
  return try_infer_impl(expr, *this);
}

}  // namespace bython::type_system