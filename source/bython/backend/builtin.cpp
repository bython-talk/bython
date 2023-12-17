#include <iostream>
#include <ranges>

#include "builtin.hpp"

namespace builtin
{
auto put_u64_impl(uint64_t value) -> void
{
  std::cout << value;
}


auto put_i64_impl(int64_t value) -> void
{
  std::cout << value;
}

auto put_f32_impl(float value) -> void
{
  std::cout << value;
}

auto putln_i64_impl(int64_t value) -> void
{
  std::cout << value << "\n";
}

}  // namespace builtin

namespace
{
using namespace bython::backend;

using builtin_factory = llvm::FunctionType* (*)(llvm::LLVMContext&);

struct table_entry
{
  builtin_tag tag;
  std::string_view name;
  builtin_factory factory;
  std::uint64_t procedure_addr;
};

static auto const builtin_lookup = std::array {
    // void @bython.put_i64(i64)
    table_entry {.tag = builtin_tag::put_i64,
                 .name = "bython.put_i64",
                 .factory = [](llvm::LLVMContext& context) -> llvm::FunctionType*
                 {
                   return llvm::FunctionType::get(
                       /*Result=*/llvm::Type::getVoidTy(context),
                       /*Params=*/ {llvm::Type::getInt64Ty(context)},
                       /*IsVarArg=*/false);
                 },
                 .procedure_addr = std::uint64_t(builtin::put_i64_impl)},

                 table_entry {.tag = builtin_tag::put_u64,
                 .name = "bython.put_u64",
                 .factory = [](llvm::LLVMContext& context) -> llvm::FunctionType*
                 {
                   return llvm::FunctionType::get(
                       /*Result=*/llvm::Type::getVoidTy(context),
                       /*Params=*/ {llvm::Type::getInt64Ty(context)},
                       /*IsVarArg=*/false);
                 },
                 .procedure_addr = std::uint64_t(builtin::put_i64_impl)},

    // void @bython.put_f32(f32)
    table_entry {.tag = builtin_tag::put_f32,
                 .name = "bython.put_f32",
                 .factory = [](llvm::LLVMContext& context) -> llvm::FunctionType*
                 {
                   return llvm::FunctionType::get(
                       /*Result=*/llvm::Type::getVoidTy(context),
                       /*Params=*/ {llvm::Type::getFloatTy(context)},
                       /*IsVarArg=*/false);
                 },
                 .procedure_addr = std::uint64_t(builtin::put_f32_impl)},

    // void @bython.putln_i64(i64)
    table_entry {.tag = builtin_tag::putln_i64,
                 .name = "bython.putln_i64",
                 .factory = [](llvm::LLVMContext& context) -> llvm::FunctionType*
                 {
                   return llvm::FunctionType::get(
                       /*Result=*/llvm::Type::getVoidTy(context),
                       /*Params=*/ {llvm::Type::getInt64Ty(context)},
                       /*IsVarArg=*/false);
                 },
                 .procedure_addr = std::uint64_t(builtin::putln_i64_impl)},
};
}  // namespace

namespace bython::backend
{

auto builtin(llvm::LLVMContext& context, builtin_tag btag) -> builtin_metadata
{
  auto entry = builtin_lookup[std::underlying_type_t<builtin_tag>(btag)];
  return builtin_metadata {.name = entry.name,
                           .signature = entry.factory(context),
                           .procedure_addr = entry.procedure_addr};
}

auto builtin(llvm::LLVMContext& context, std::string_view name) -> std::optional<builtin_metadata>
{
  auto desc =
      std::ranges::find_if(builtin_lookup, [&name](auto&& entry) { return entry.name == name; });
  if (desc != builtin_lookup.end()) {
    return builtin_metadata {.name = desc->name,
                             .signature = desc->factory(context),
                             .procedure_addr = desc->procedure_addr};
  }

  return std::nullopt;
}

}  // namespace bython::backend