#pragma once

#include <memory>
#include <optional>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "builtin.hpp"

namespace bython::type_system
{
using subtype_converter = llvm::Value* (*)(llvm::IRBuilder<>& builder,
                                           llvm::Value* expr,
                                           llvm::Type* dest);

auto try_subtype_impl(type const& tau, type const& alpha) -> std::optional<subtype_converter>;
}  // namespace bython::type_system