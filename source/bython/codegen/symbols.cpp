#include "symbols.hpp"

namespace bython::codegen
{

auto symbol_lookup::get(std::string_view symbol_name) const
    -> std::optional<std::tuple<llvm::Type*, llvm::Value*>>
{
  for (auto it = this->lookup.rbegin(); it != this->lookup.rend(); ++it) {
    if (auto search = it->find(symbol_name); search != it->end()) {
      return search->second;
    }
  }

  // TODO: Builtins can go here; manual ChainMap style

  return std::nullopt;
}

auto symbol_lookup::put(std::string_view symbol_name,
                        llvm::Type* symbol_type,
                        llvm::Value* symbol_storage_loc) -> void
{
  this->lookup.back().insert_or_assign(symbol_name,
                                       std::make_pair(symbol_type, symbol_storage_loc));
}

auto symbol_lookup::update(std::string_view symbol_name, llvm::Value* symbol_storage_loc) -> void
{
  for (auto it = this->lookup.rbegin(); it != this->lookup.rend(); ++it) {
    if (auto search = it->find(symbol_name); search != it->end()) {
      std::get<1>(search->second) = symbol_storage_loc;
      break;
    }
  }

  // Do not check builtins; they are immutable
}

auto symbol_lookup::initialise_new_scope() -> void
{
  this->lookup.emplace_back();
}

auto symbol_lookup::pop_scope() -> void
{
  this->lookup.pop_back();
}

auto type_lookup::get(std::string_view identifier) const -> std::optional<llvm::Type*>
{
  for (auto it = this->lookup.rbegin(); it != this->lookup.rend(); ++it) {
    if (auto search = it->find(identifier); search != it->end()) {
      return search->second;
    }
  }

  return std::nullopt;
}

auto type_lookup::put(std::string_view symbol_name, llvm::Type* symbol_type) -> void
{
  this->lookup.back().insert_or_assign(symbol_name, symbol_type);
}

auto type_lookup::initialise_new_scope() -> void
{
  this->lookup.emplace_back();
}

auto type_lookup::pop_scope() -> void
{
  this->lookup.pop_back();
}

}  // namespace bython::codegen