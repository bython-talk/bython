#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <vector>

#include <boost/uuid/uuid.hpp>

#include "builtin.hpp"
#include "bython/ast/expression.hpp"
#include "bython/ast/statement.hpp"
#include "bython/type_system/subtyping.hpp"

namespace bython::type_system
{
class environment
{
  std::unordered_set<std::unique_ptr<type_system::type>> m_visible_types;
  std::map<std::string, type_system::type*, std::less<>> m_typename_to_typeptr;

  std::map<std::string, type_system::type*, std::less<>> m_symbol_to_ts;

public:
  static auto initialise_with_builtins() -> environment;

  auto add_new_symbol(std::string sname, type_system::type* type) -> void;
  auto add_new_named_type(std::string tname, std::unique_ptr<type_system::type> type)
      -> type_system::type*;
  auto add_new_function_type(ast::function_def const& function)
      -> std::optional<type_system::function*>;

  auto lookup_type(std::string_view tname) const -> std::optional<type_system::type*>;
  // auto lookup(ast::expression const& expr) const -> std::optional<type_system::type*>;

  auto get_type(ast::expression const& expr) const -> std::optional<type_system::type*>;

  auto try_subtype(type_system::type const& tau, type_system::type const& alpha) const
      -> std::optional<type_system::subtyping_rule>;
};
}  // namespace bython::type_system
