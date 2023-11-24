#include "builtin.hpp"

// #include <llvm/ADT/ArrayRef.h>
// #include <llvm/ADT/StringRef.h>
// #include <llvm/ADT/Twine.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

namespace bython::type_system
{
auto type::operator!=(type const& other) const -> bool
{
  return !(*this == other);
}

auto uint::definition(llvm::LLVMContext& context) const -> llvm::Type*
{
  static const auto underlying = llvm::Type::getIntNTy(context, this->width);
  return underlying;
}

auto uint::tag() const -> type_tag
{
  return type_tag::uint;
}

auto uint::operator==(type const& other) const -> bool
{
  auto const* other_uint = dynamic_cast<uint const*>(&other);
  return other_uint != nullptr && this->width == other_uint->width;
}

auto sint::definition(llvm::LLVMContext& context) const -> llvm::Type*
{
  static const auto underlying = llvm::Type::getIntNTy(context, this->width);
  return underlying;
}

auto sint::tag() const -> type_tag
{
  return type_tag::sint;
}

auto sint::operator==(type const& other) const -> bool
{
  auto const* other_sint = dynamic_cast<sint const*>(&other);
  return other_sint != nullptr && this->width == other_sint->width;
}

auto single_fp::definition(llvm::LLVMContext& context) const -> llvm::Type*
{
  static const auto single = llvm::Type::getFloatTy(context);
  return single;
}

auto single_fp::tag() const -> type_tag
{
  return type_tag::single_fp;
}

auto single_fp::operator==(type const& other) const -> bool
{
  auto const* other_single = dynamic_cast<single_fp const*>(&other);
  return other_single != nullptr;
}

auto double_fp::definition(llvm::LLVMContext& context) const -> llvm::Type*
{
  static const auto dbl = llvm::Type::getDoubleTy(context);
  return dbl;
}

auto double_fp::tag() const -> type_tag
{
  return type_tag::double_fp;
}

auto double_fp::operator==(type const& other) const -> bool
{
  auto const* other_double_fp = dynamic_cast<double_fp const*>(&other);
  return other_double_fp != nullptr;
}

}  // namespace bython::type_system