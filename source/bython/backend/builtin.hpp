#pragma once

#include <cinttypes>
#include <optional>
#include <string_view>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>

namespace bython::backend
{

struct builtin_metadata
{
  std::string_view name;
  llvm::FunctionType* signature;
  std::uint64_t procedure_addr;
};

enum class builtin_tag : std::uint8_t
{
  put_i64,
  put_u64,
  putln_i64,

  put_f32,
};

auto builtin(llvm::LLVMContext& context, builtin_tag btag) -> builtin_metadata;
auto builtin(llvm::LLVMContext& context, std::string_view btag) -> std::optional<builtin_metadata>;

}  // namespace bython::backend
