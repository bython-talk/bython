#pragma once

#include <cinttypes>
#include <optional>
#include <string_view>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>

#include "bython/type_system/builtin.hpp"

namespace bython::backend
{

struct builtin_metadata
{
  std::string_view name;
  llvm::FunctionType* signature;
  std::uint64_t procedure_addr;
};

auto builtin_function(llvm::LLVMContext& context, type_system::function_tag ftag) -> builtin_metadata;
auto type(llvm::LLVMContext& context, type_system::type const& type) -> llvm::Type*;

}  // namespace bython::backend
