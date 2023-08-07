#include <ranges>
#include <unordered_map>

#include "symbols.hpp"

#include <llvm/IR/DerivedTypes.h>

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
  for (auto&& entry : std::ranges::reverse_view(this->lookup)) {
    if (auto search = entry.find(symbol_name); search != entry.end()) {
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

using type_factory = llvm::Type* (*)(llvm::LLVMContext&);

static auto builtin_types = std::unordered_map<std::string_view, type_factory> {
    // signed integer types
    {"i64", [](llvm::LLVMContext& c) { return cast<llvm::Type>(llvm::Type::getInt64Ty(c)); }},
    {"i32", [](llvm::LLVMContext& c) { return cast<llvm::Type>(llvm::Type::getInt32Ty(c)); }},
    {"i16", [](llvm::LLVMContext& c) { return cast<llvm::Type>(llvm::Type::getInt16Ty(c)); }},
    {"i8", [](llvm::LLVMContext& c) { return cast<llvm::Type>(llvm::Type::getInt8Ty(c)); }},

    // floating point types
    {"f64", [](llvm::LLVMContext& c) { return cast<llvm::Type>(llvm::Type::getDoubleTy(c)); }},
    {"f32", [](llvm::LLVMContext& c) { return cast<llvm::Type>(llvm::Type::getFloatTy(c)); }}};

auto type_lookup::get(llvm::LLVMContext& context, std::string_view identifier) const
    -> std::optional<llvm::Type*>
{
  for (auto&& entry : std::ranges::reverse_view(this->lookup)) {
    if (auto search = entry.find(identifier); search != entry.end()) {
      return search->second;
    }
  }

  if (auto search = builtin_types.find(identifier); search != builtin_types.end()) {
    return search->second(context);
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