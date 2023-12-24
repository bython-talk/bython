#include <optional>
#include <stdexcept>
#include <vector>

#include "typing.hpp"

#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "bython/type_system/builtin.hpp"
#include "bython/type_system/subtyping.hpp"

namespace ts = bython::type_system;



namespace bython::backend
{

auto subtype_conversion(ts::subtyping_rule rule) -> subtype_converter
{
  switch (rule) {
    case ts::subtyping_rule::identity: {
      return [](llvm::IRBuilder<>& /*builder*/,
                llvm::Value* expr,
                llvm::Type* /*dest*/) -> llvm::Value* { return expr; };
    }
    case type_system::subtyping_rule::bool_int_prom:
    case type_system::subtyping_rule::uint_promotion: {
      return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest) -> llvm::Value*
      { return builder.CreateIntCast(expr, dest, /*isSigned=*/false, "uint.prom"); };
    }

    case type_system::subtyping_rule::sint_promotion: {
      return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest) -> llvm::Value*
      { return builder.CreateIntCast(expr, dest, /*isSigned=*/true, "sint.prom"); };
    }

    case type_system::subtyping_rule::bool_fp_prom:
    case type_system::subtyping_rule::uint_to_single:
    case type_system::subtyping_rule::uint_to_double: {
      return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest) -> llvm::Value*
      { return builder.CreateUIToFP(expr, dest, "ui2fp.prom"); };
    }
    case type_system::subtyping_rule::sint_to_single:
    case type_system::subtyping_rule::sint_to_double:
      return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest) -> llvm::Value*
      { return builder.CreateSIToFP(expr, dest, "si2fp.prom"); };

    case type_system::subtyping_rule::single_to_double: {
      return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest) -> llvm::Value*
      { return builder.CreateFPExt(expr, dest, "fp.prom"); };
    }
    case type_system::subtyping_rule::numeric_to_bool: {
      return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* /*dest*/) -> llvm::Value*
      {
        auto ety = expr->getType();
        if (ety->isFloatingPointTy()) {
          return builder.CreateFCmpUNE(expr, llvm::ConstantFP::get(ety, 0.0));
        }
        return builder.CreateICmpNE(expr, llvm::ConstantInt::get(ety, 0, /*IsSigned=*/false));
      };
      return [](llvm::IRBuilder<>& builder, llvm::Value* expr, llvm::Type* dest) -> llvm::Value*
      { return builder.CreateFPExt(expr, dest, "bool.fp.prom"); };
    }
  }
}

}  // namespace bython::backend