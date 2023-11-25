#include <ranges>
#include <unordered_map>

#include "symbols.hpp"

#include <llvm/IR/DerivedTypes.h>

namespace bython::codegen
{

auto symbol_lookup::get(std::string_view symbol_name) const
    -> std::optional<std::tuple<llvm::Type*, llvm::Value*>>
{
  for (const auto& it : std::ranges::reverse_view(this->lookup)) {
    if (auto search = it.find(symbol_name); search != it.end()) {
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

using type_factory = metadata (*)(llvm::LLVMContext&);

static auto builtin_types = std::unordered_map<std::string_view, type_factory> {
    // signed integer type
    {"i64",
     [](llvm::LLVMContext& c)
     { return metadata {.type = llvm::Type::getInt64Ty(c), .is_signed = true}; }},

    // unsigned integer
    {"u64",
     [](llvm::LLVMContext& c)
     { return metadata {.type = llvm::Type::getInt64Ty(c), .is_signed = false}; }},

    // boolean type
    {"bool",
     [](llvm::LLVMContext& c)
     { return metadata {.type = llvm::Type::getInt1Ty(c), .is_signed = false}; }},

    // floating point
    {"f64", [](llvm::LLVMContext& c) {
       return metadata {.type = llvm::Type::getDoubleTy(c), .is_signed = false};
     }}};

auto type_lookup::get(llvm::LLVMContext& context, std::string_view identifier) const
    -> std::optional<metadata>
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
  this->lookup.back().insert_or_assign(symbol_name,
                                       metadata {.type = symbol_type, .is_signed = std::nullopt});
}

auto type_lookup::put(std::string_view symbol_name, llvm::Type* symbol_type, bool is_signed) -> void
{
  this->lookup.back().insert_or_assign(symbol_name,
                                       metadata {.type = symbol_type, .is_signed = is_signed});
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