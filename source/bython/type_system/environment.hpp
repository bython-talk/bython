#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include <boost/uuid/uuid.hpp>

#include "builtin.hpp"
#include "bython/ast/expression.hpp"
#include "bython/type_system/subtyping.hpp"

namespace bython::type_system
{
class environment
{
  std::vector<std::unique_ptr<type_system::type>> m_visible_types;
  std::map<std::string, type_system::type*, std::less<>> m_typename_to_typeptr;

  std::map<std::string, type_system::type*, std::less<>> m_symbol_to_ts;

public:
  static auto initialise_with_builtins() -> environment;

  auto add(std::string tname, std::unique_ptr<type> type) -> void;
  auto lookup(std::string_view tname) const -> std::optional<type_system::type*>;
  // auto lookup(ast::expression const& expr) const -> std::optional<type_system::type*>;

  auto infer(ast::expression const& expr) const -> std::optional<type_system::type*>;

  auto try_subtype(type_system::type const& tau, type_system::type const& alpha) const
      -> std::optional<type_system::subtyping_rule>;
};
}  // namespace bython::type_system
