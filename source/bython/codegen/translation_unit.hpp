#pragma once

#include <bython/ast/module.hpp>
#include <llvm/IR/Module.h>

#include "bases.hpp"

namespace bython::codegen
{
struct translation_unit : codegenable
{
  auto into_module() const -> std::unique_ptr<llvm::Module> override;

  ast::mod module_;
};

/*
struct translation_project : codegenable
{
  auto into_module() const -> std::unique_ptr<llvm::Module> override;

  std::map<std::string, std::unique_ptr<ast::node>> file2code;
};
*/

}  // namespace bython::codegen