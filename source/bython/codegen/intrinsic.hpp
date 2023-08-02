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
  // io
  put_i64,

  // maths
  powi_f32_i32
};

auto builtin_intrinsic(llvm::LLVMContext& context, intrinsic_tag itag) -> intrinsic;

}  // namespace bython::codegen