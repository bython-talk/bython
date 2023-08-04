#pragma once

#include <filesystem>
#include <memory>
#include <string_view>

#include <bython/ast/bases.hpp>

namespace bython::executor
{
struct jit_compiler
{
  jit_compiler();
  ~jit_compiler();

  jit_compiler(jit_compiler const&) = delete;
  auto operator=(jit_compiler const&) noexcept -> jit_compiler& = delete;

  jit_compiler(jit_compiler&&);
  auto operator=(jit_compiler&&) noexcept -> jit_compiler&;

  auto execute(std::filesystem::path const& input_file) -> int;

private:
  struct jit_compiler_pimpl;
  std::unique_ptr<jit_compiler_pimpl> impl;
};

}  // namespace bython::codegen