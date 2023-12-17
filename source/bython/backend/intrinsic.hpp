#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>

namespace bython::backend
{

struct intrinsic_metadata
{
  std::string_view name;
  llvm::FunctionType* signature;
};

enum class intrinsic_tag : std::uint8_t
{
  // maths
  powi_f32_i32
};

auto intrinsic(llvm::LLVMContext& context, intrinsic_tag itag) -> intrinsic_metadata;
auto intrinsic(llvm::LLVMContext& context, std::string_view itag)
    -> std::optional<intrinsic_metadata>;

}  // namespace bython::backend