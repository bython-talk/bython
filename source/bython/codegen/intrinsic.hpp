#pragma once

#include <string>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>

namespace bython::codegen
{

struct intrinsic_metadata
{
  std::string name;
  llvm::FunctionType* signature;
};

enum class intrinsic_tag {
  // io
  put_i64,

  // maths
  powi_f32_i32
};

auto intrinsic(llvm::LLVMContext& context, intrinsic_tag itag) -> intrinsic_metadata;

}  // namespace bython::codegen