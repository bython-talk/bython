#include <functional>
#include <unordered_map>

#include "intrinsic.hpp"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/ErrorHandling.h>

namespace
{
using namespace bython::codegen;

using intrinsic_factory = llvm::FunctionType* (*)(llvm::LLVMContext&);
struct table_entry
{
  intrinsic_tag tag;
  std::string_view name;
  intrinsic_factory factory;
};

static auto const intrinsic_lookup = std::array {
    // void @bython.put_i64(i64)
    table_entry {
        .tag = intrinsic_tag::powi_f32_i32,
        .name = "llvm.powi.f32.i32",
        .factory = [](llvm::LLVMContext& context) -> llvm::FunctionType*
        {
          return llvm::FunctionType::get(
              /*Result=*/llvm::Type::getFloatTy(context),
              /*Params=*/ {llvm::Type::getFloatTy(context), llvm::Type::getInt32Ty(context)},
              /*IsVarArg=*/false);
        }}};
}  // namespace

namespace bython::codegen
{

auto intrinsic(llvm::LLVMContext& context, intrinsic_tag itag) -> intrinsic_metadata
{
  auto entry = intrinsic_lookup[std::underlying_type_t<intrinsic_tag>(itag)];
  return intrinsic_metadata {.name = entry.name, .signature = entry.factory(context)};
}

auto intrinsic(llvm::LLVMContext& context, std::string_view name)
    -> std::optional<intrinsic_metadata>
{
  auto desc =
      std::ranges::find_if(intrinsic_lookup, [&name](auto&& entry) { return entry.name == name; });
  if (desc != intrinsic_lookup.end()) {
    return intrinsic_metadata {.name = desc->name, .signature = desc->factory(context)};
  }

  return std::nullopt;
}

}  // namespace bython::codegen