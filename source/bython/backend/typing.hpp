#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "bython/type_system/builtin.hpp"
#include "bython/type_system/environment.hpp"

namespace bython::backend
{
auto definition(llvm::LLVMContext& context, type_system::type const& type) -> llvm::Type*;

using subtype_converter = llvm::Value* (*)(llvm::IRBuilder<>& builder,
                                           llvm::Value* expr,
                                           llvm::Type* dest);
auto subtype_conversion(type_system::subtyping_rule rule) -> subtype_converter;
}  // namespace bython::backend