#pragma once

#include <map>
#include <optional>
#include <vector>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace bython::codegen
{
class symbol_lookup
{
  using mapping = std::unordered_map<std::string_view, std::tuple<llvm::Type*, llvm::Value*>>;

public:
  symbol_lookup() { this->lookup.emplace_back(); }

  auto get(std::string_view identifier) const
      -> std::optional<std::tuple<llvm::Type*, llvm::Value*>>;

  auto put(std::string_view symbol_name, llvm::Type* symbol_type, llvm::Value* symbol_storage_loc)
      -> void;

  auto update(std::string_view symbol_name, llvm::Value* symbol_storage_loc) -> void;

  auto initialise_new_scope() -> void;
  auto pop_scope() -> void;

private:
  std::vector<mapping> lookup;
};

class type_lookup
{
  using mapping = std::unordered_map<std::string_view, llvm::Type*>;

public:
  type_lookup() { this->lookup.emplace_back(); }

  auto get(std::string_view identifier) const -> std::optional<llvm::Type*>;

  auto put(std::string_view symbol_name, llvm::Type* symbol_type) -> void;

  auto initialise_new_scope() -> void;
  auto pop_scope() -> void;

private:
  std::vector<mapping> lookup;
};

}  // namespace bython::codegen