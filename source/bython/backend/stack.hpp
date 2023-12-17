#pragma once

#include <map>
#include <optional>
#include <vector>

#include <llvm-16/llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace bython::backend
{

class stack
{
  using mapping = std::map<std::string, llvm::Value*, std::less<>>;

public:
  stack();

  auto get(std::string_view identifier) const -> std::optional<llvm::Value*>;
  auto put(std::string symbol_name, llvm::Value* symbol_storage_loc) -> void;

  auto push_new_scope() -> void;
  auto pop_scope() -> void;

private:
  std::vector<mapping> lookup;
};
}  // namespace bython::backend