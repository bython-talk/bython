#pragma once

#include <string>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>

namespace bython::codegen
{

struct intrinsic
{
  std::string name;
  llvm::FunctionType* signature;
};

enum class intrinsic_tag {
  powi_f32_i32
};

auto builtin_intrinsic(llvm::LLVMContext& context, intrinsic_tag itag) -> intrinsic;

auto powi_f32_i32(llvm::LLVMContext& context) -> intrinsic;

}  // namespace bython::codegen