#pragma once

#include <bython/ast/bases.hpp>
#include <llvm/IR/Module.h>

namespace bython::codegen
{
auto compile(std::string_view name, std::unique_ptr<ast::node> ast, llvm::LLVMContext& context)
    -> std::unique_ptr<llvm::Module>;

auto compile(std::unique_ptr<ast::node> ast,
             llvm::Module& module_) -> void;
}  // namespace bython::codegen