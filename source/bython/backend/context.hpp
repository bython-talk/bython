#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "symbols.hpp"
#include "type_system.hpp"

namespace bython::codegen
{
struct codegen_context
{
  llvm::LLVMContext& context;
  llvm::IRBuilder<> builder;
  llvm::Module& module_;

  codegen::symbol_lookup symbol_mapping;
  codegen::type_lookup type_mapping;
};
}  // namespace bython::codegen