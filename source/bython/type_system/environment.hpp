#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include "builtin.hpp"
#include "bython/ast/expression.hpp"
#include "bython/type_system/subtyping.hpp"

namespace bython::type_system
{
class environment
{
  std::vector<std::unique_ptr<type_system::type>> m_visible_types;

  std::map<std::string, type_system::type const*, std::less<>> m_id_to_ts;
  std::map<boost::uuids::uuid, type_system::type const*, std::less<>> m_expr_to_ts;

public:
  static auto initialise_with_builtins() -> environment;

  auto add(std::string tname, std::unique_ptr<type> type) -> void;
  auto lookup(std::string_view tname) const -> std::optional<type_system::type const*>;
  auto lookup(ast::expression const& expr) const -> std::optional<type_system::type const*>;

  auto typeify_expression(ast::expression const& expr, std::string_view tname) -> std::optional<type_system::type const*>;
  auto infer(ast::expression const& expr) const -> std::optional<type_system::type const*>;

  auto try_subtype(type_system::type const& tau, type_system::type const& alpha) -> std::optional<type_system::subtype_converter>;

private:
};
}  // namespace bython::type_system
