#pragma once

#include <optional>
#include <tuple>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Value.h>

namespace bython::codegen
{

auto convert(llvm::IRBuilder<>& builder, llvm::Value* src, llvm::Type* dst_type)
    -> std::optional<llvm::Value*>;

auto unify(llvm::IRBuilder<>& builder, llvm::Value* lhs, llvm::Value* rhs)
    -> std::optional<std::tuple<llvm::Value*, llvm::Value*>>;
}  // namespace bython::codegen