#include <optional>
#include <stdexcept>
#include <vector>

#include "typing.hpp"

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>

#include "bython/type_system/builtin.hpp"
#include "bython/type_system/subtyping.hpp"

namespace ts = bython::type_system;

namespace
{
auto definition_impl(llvm::LLVMContext& context, ts::void_ const& /*void*/) -> llvm::Type*
{
  return llvm::Type::getVoidTy(context);
}

auto definition_impl(llvm::LLVMContext& context, ts::boolean const& /*boolean*/)
    -> llvm::IntegerType*
{
  return llvm::Type::getInt1Ty(context);
}

auto definition_impl(llvm::LLVMContext& context, ts::uint const& unsigned_int) -> llvm::IntegerType*
{
  return llvm::Type::getIntNTy(context, unsigned_int.width);
}

auto definition_impl(llvm::LLVMContext& context, ts::sint const& signed_int) -> llvm::IntegerType*
{
  return llvm::Type::getIntNTy(context, signed_int.width);
}

auto definition_impl(llvm::LLVMContext& context, ts::single_fp const& /*f32*/) -> llvm::Type*
{
  return llvm::Type::getFloatTy(context);
}

auto definition_impl(llvm::LLVMContext& context, ts::double_fp const& /*f32*/) -> llvm::Type*
{
  return llvm::Type::getDoubleTy(context);
}

auto definition_impl(llvm::LLVMContext& context, ts::func_sig const& func) -> llvm::FunctionType*
{
  auto params = std::vector<llvm::Type*> {};
  for (auto&& fparam : func.parameters) {
    params.emplace_back(bython::backend::definition(context, *fparam));
  }

  auto rettype = bython::backend::definition(context, *func.rettype);
  return llvm::FunctionType::get(rettype, params, /*isVarArg=*/false);
}
}  // namespace

namespace bython::backend
{

auto definition(llvm::LLVMContext& context, ts::type const& type) -> llvm::Type*
{
  switch (type.tag()) {
    // TODO: Improve error handling
    case ts::type_tag::void_:
      return definition_impl(context, dynamic_cast<ts::void_ const&>(type));

    case ts::type_tag::boolean:
      return definition_impl(context, dynamic_cast<ts::boolean const&>(type));

    case ts::type_tag::uint:
      return definition_impl(context, dynamic_cast<ts::uint const&>(type));

    case ts::type_tag::sint:
      return definition_impl(context, dynamic_cast<ts::sint const&>(type));

    case ts::type_tag::single_fp:
      return definition_impl(context, dynamic_cast<ts::single_fp const&>(type));

    case ts::type_tag::double_fp:
      return definition_impl(context, dynamic_cast<ts::double_fp const&>(type));

    case ts::type_tag::function:
      return definition_impl(context, dynamic_cast<ts::func_sig const&>(type));
  }
}

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