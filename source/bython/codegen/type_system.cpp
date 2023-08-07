#include <algorithm>
#include <numeric>
#include <vector>

#include "type_system.hpp"

namespace
{
struct converter
{
  virtual ~converter() = default;

  virtual auto codegen(llvm::IRBuilder<>& builder, llvm::Value* src) -> llvm::Value* = 0;
};

struct chained_converter final : converter
{
  explicit chained_converter(std::vector<std::unique_ptr<converter>> converters_)
      : converters {std::move(converters_)}
  {
  }

  auto codegen(llvm::IRBuilder<>& builder, llvm::Value* src) -> llvm::Value*
  {
    for (auto&& converter : this->converters) {
      src = converter->codegen(builder, src);
    }
    return src;
  }

  std::vector<std::unique_ptr<converter>> converters;
};

struct type_converter : converter
{
  explicit type_converter(llvm::Type* dst_type_)
      : dst_type {dst_type_}
  {
  }

  llvm::Type* dst_type;
};

struct int_promoter final : type_converter
{
  using type_converter::type_converter;

  auto codegen(llvm::IRBuilder<>& builder, llvm::Value* src) -> llvm::Value*
  {
    return builder.CreateIntCast(src, this->dst_type, /*isSigned=*/true, "int.prom");
  }
};

struct int_truncater final : type_converter
{
  using type_converter::type_converter;

  auto codegen(llvm::IRBuilder<>& builder, llvm::Value* src) -> llvm::Value*
  {
    // Produce true iff integer value is not 0
    return builder.CreateICmpNE(src, llvm::ConstantInt::get(src->getType(), 0, /*isSigned=*/true), "int.trunc");
  }
};

struct int_to_fp final : type_converter
{
  using type_converter::type_converter;

  auto codegen(llvm::IRBuilder<>& builder, llvm::Value* src) -> llvm::Value*
  {
    return builder.CreateSIToFP(src, this->dst_type, "int2fp.prom");
  }
};

}  // namespace

namespace bython::codegen
{
auto convert(llvm::IRBuilder<>& builder, llvm::Value* src, llvm::Type* dst_type)
    -> std::optional<llvm::Value*>
{
  if (src->getType()->isIntegerTy() && dst_type->isIntegerTy()) {
    if (src->getType()->getIntegerBitWidth() <= dst_type->getIntegerBitWidth()) {
      auto converter = int_promoter {dst_type};
      return converter.codegen(builder, src);

    }

    else
    {
      auto converter = int_truncater {dst_type};
      return converter.codegen(builder, src);
    }
  }

  else if (src->getType()->isIntegerTy() && dst_type->isFloatingPointTy())
  {
    // TODO: Check losslessness
    auto converter = int_to_fp{dst_type};
    return converter.codegen(builder, src);

  }

  return std::nullopt;
}
}  // namespace bython::codegen