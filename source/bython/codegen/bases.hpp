#pragma once

#include <memory>

namespace bython::codegen
{
struct codegenable
{
  virtual ~codegenable() = default;

  virtual auto into_module() const -> std::unique_ptr<llvm::Module> = 0;
};
}  // namespace bython::codegen