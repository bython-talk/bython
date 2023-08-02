#include "intrinsic.hpp"

#include <llvm/IR/DerivedTypes.h>

namespace bython::codegen
{

auto put_i64(llvm::LLVMContext& context) -> intrinsic
{
  return intrinsic {"bython.put_i64",
                    llvm::FunctionType::get(
                        /*Result=*/llvm::Type::getVoidTy(context),
                        /*Params=*/ {llvm::Type::getInt64Ty(context)},
                        /*IsVarArg=*/false)};
}

auto powi_f32_i32(llvm::LLVMContext& context) -> intrinsic
{
  return intrinsic {
      "llvm.powi.f32.i32",
      llvm::FunctionType::get(
          /*Result=*/llvm::Type::getFloatTy(context),
          /*Params=*/ {llvm::Type::getFloatTy(context), llvm::Type::getInt32Ty(context)},
          /*IsVarArg=*/false)};
}

auto builtin_intrinsic(llvm::LLVMContext& context, intrinsic_tag itag) -> intrinsic
{
  switch (itag) {
    // io
    case intrinsic_tag::put_i64:
      return put_i64(context);

    // math
    case intrinsic_tag::powi_f32_i32:
      return powi_f32_i32(context);
    default:
      llvm_unreachable("Unrecognised intrinsic type");
  }
}

}  // namespace bython::codegen