#pragma once

#include "bases.hpp"

namespace bython::codegen
{
struct stdin : codegenable
{
  auto into_module() const -> std::unique_ptr<llvm::Module> override;

  auto execute() const -> void;
};
}  // namespace bython::codegen