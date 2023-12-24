#pragma once

#include <bython/ast/bases.hpp>
#include <bython/frontend/frontend.hpp>
#include <llvm/IR/Module.h>

namespace bython::backend
{
auto compile(std::string_view name,
             std::unique_ptr<ast::node> ast,
             parser::parse_metadata const& metadata,
             llvm::LLVMContext& context) -> std::unique_ptr<llvm::Module>;

auto compile(std::unique_ptr<ast::node> ast,
             parser::parse_metadata const& metadata,
             llvm::Module& module_) -> void;
}  // namespace bython::backend