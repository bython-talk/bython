#include <ranges>
#include <unordered_map>

#include "stack.hpp"

#include <llvm/IR/DerivedTypes.h>

namespace bython::backend
{

stack::stack()
{
  this->lookup.emplace_back();
}

auto stack::get(std::string_view symbol_name) const -> std::optional<llvm::Value*>
{
  for (auto&& it : std::ranges::reverse_view(this->lookup)) {
    if (auto search = it.find(symbol_name); search != it.end()) {
      return search->second;
    }
  }

  return std::nullopt;
}

auto stack::put(std::string symbol_name, llvm::Value* symbol_storage_loc) -> void
{
  this->lookup.back().insert_or_assign(symbol_name, symbol_storage_loc);
}

auto stack::push_new_scope() -> void
{
  this->lookup.emplace_back();
}

auto stack::pop_scope() -> void
{
  this->lookup.pop_back();
}

}  // namespace bython::backend