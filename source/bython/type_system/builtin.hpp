#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>

namespace bython::type_system
{
enum class type_tag
{
  uint,
  sint,
  single_fp,
  double_fp,
};

struct type
{
  type() = default;
  virtual ~type() = default;

  type(const type&) = default;
  auto operator=(const type&) -> type& = default;

  type(type&&) = delete;
  auto operator=(type&&) -> type& = delete;

  virtual auto operator==(type const& other) const -> bool;
  virtual auto operator!=(type const& other) const -> bool final;

  virtual auto tag() const -> type_tag;
  virtual auto definition(llvm::LLVMContext& context) const -> llvm::Type*;
};

struct uint final : type
{
  uint(unsigned width);

  unsigned width;
  auto operator==(type const& other) const -> bool;

  auto tag() const -> type_tag;
  auto definition(llvm::LLVMContext& context) const -> llvm::Type*;
};

struct sint final : type
{
  sint(unsigned width);

  unsigned width;
  auto operator==(type const& other) const -> bool;

  auto tag() const -> type_tag;
  auto definition(llvm::LLVMContext& context) const -> llvm::Type*;
};

struct single_fp final : type
{
  auto operator==(type const& other) const -> bool;

  auto tag() const -> type_tag;
  auto definition(llvm::LLVMContext& context) const -> llvm::Type*;
};

struct double_fp final : type
{
  auto operator==(type const& other) const -> bool;

  auto tag() const -> type_tag;
  auto definition(llvm::LLVMContext& context) const -> llvm::Type*;
};

}  // namespace bython::type_system