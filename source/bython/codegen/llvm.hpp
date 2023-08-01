#pragma once

#include <bython/ast/bases.hpp>
#include <llvm/IR/Module.h>

namespace bython::codegen
{
auto compile(std::unique_ptr<ast::node> ast) -> std::unique_ptr<llvm::Module>;
}