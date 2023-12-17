#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "environment.hpp"

#include <boost/uuid/uuid_io.hpp>

#include "builtin.hpp"
#include "bython/ast.hpp"
#include "bython/ast/visitor.hpp"
#include "bython/type_system/inference.hpp"
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

  // void / unit
  env.add_new_named_type("void", std::make_unique<void_>());

  // boolean
  env.add_new_named_type("bool", std::make_unique<boolean>());

  // uint{8, 16, 32, 64}_t
  env.add_new_named_type("u8", std::make_unique<uint>(8));
  env.add_new_named_type("u16", std::make_unique<uint>(16));
  env.add_new_named_type("u32", std::make_unique<uint>(32));
  env.add_new_named_type("u64", std::make_unique<uint>(64));

  // int{8, 16, 32, 64}_t
  env.add_new_named_type("i8", std::make_unique<sint>(8));
  env.add_new_named_type("i16", std::make_unique<sint>(16));
  env.add_new_named_type("i32", std::make_unique<sint>(32));
  env.add_new_named_type("i64", std::make_unique<sint>(64));

  // f{32, 64}
  env.add_new_named_type("f32", std::make_unique<single_fp>());
  env.add_new_named_type("f64", std::make_unique<double_fp>());

  return env;
}

auto environment::add_new_symbol(std::string sname, type_system::type* type) -> void
{
  this->m_symbol_to_ts.emplace(sname, type);
}

auto environment::add_new_named_type(std::string tname, std::unique_ptr<type> type)
    -> type_system::type*
{
  auto&& [inserted, _] = this->m_visible_types.emplace(std::move(type));
  this->m_typename_to_typeptr.emplace(std::move(tname), inserted->get());
  return inserted->get();
}

auto environment::add_new_function_type(ast::function_def const& function)
    -> std::optional<type_system::function*>
{
  auto parameters = std::vector<type_system::type*> {};
  auto rettype = std::optional<type_system::type*> {};

  for (auto&& parameter : function.parameters.parameters) {
    if (auto ptype = this->lookup_type(parameter.name); ptype) {
      parameters.emplace_back(*ptype);
    } else {
      return std::nullopt;
    }
  }

  auto added_ft = this->add_new_named_type(
      boost::uuids::to_string(function.uuid),
      std::make_unique<type_system::function>(std::move(parameters), rettype));
  return std::make_optional(dynamic_cast<type_system::function*>(added_ft));
}

auto environment::lookup_type(std::string_view tname) const -> std::optional<type_system::type*>
{
  if (auto it = this->m_typename_to_typeptr.find(tname); it != this->m_typename_to_typeptr.end()) {
    return it->second;
  }
  return std::nullopt;
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