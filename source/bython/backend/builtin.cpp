#include <iostream>
#include <ranges>

#include "builtin.hpp"

#include "bython/type_system/builtin.hpp"

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

auto put_f64_impl(int64_t value) -> void
{
  std::cout << value;
}

}  // namespace builtin

namespace
{
using namespace bython::backend;
namespace ts = bython::type_system;

using builtin_factory = llvm::FunctionType* (*)(llvm::LLVMContext&);

struct table_entry
{
  ts::function_tag tag;
  std::string_view name;
  builtin_factory factory;
  std::uint64_t procedure_addr;
};

static auto const builtin_lookup = std::array {
    // void @bython.put_i64(i64)
    table_entry {.tag = ts::function_tag::put_i64,
                 .name = "put_i64",
                 .factory = [](llvm::LLVMContext& context) -> llvm::FunctionType*
                 {
                   return llvm::FunctionType::get(
                       /*Result=*/llvm::Type::getVoidTy(context),
                       /*Params=*/ {llvm::Type::getInt64Ty(context)},
                       /*IsVarArg=*/false);
                 },
                 .procedure_addr = std::uint64_t(builtin::put_i64_impl)},

    table_entry {.tag = ts::function_tag::put_u64,
                 .name = "put_u64",
                 .factory = [](llvm::LLVMContext& context) -> llvm::FunctionType*
                 {
                   return llvm::FunctionType::get(
                       /*Result=*/llvm::Type::getVoidTy(context),
                       /*Params=*/ {llvm::Type::getInt64Ty(context)},
                       /*IsVarArg=*/false);
                 },
                 .procedure_addr = std::uint64_t(builtin::put_i64_impl)},

    // void @put_f32(f32)
    table_entry {.tag = ts::function_tag::put_f32,
                 .name = "put_f32",
                 .factory = [](llvm::LLVMContext& context) -> llvm::FunctionType*
                 {
                   return llvm::FunctionType::get(
                       /*Result=*/llvm::Type::getVoidTy(context),
                       /*Params=*/ {llvm::Type::getFloatTy(context)},
                       /*IsVarArg=*/false);
                 },
                 .procedure_addr = std::uint64_t(builtin::put_f32_impl)},

    // void @put_i64(i64)
    table_entry {.tag = ts::function_tag::put_f64,
                 .name = "put_i64",
                 .factory = [](llvm::LLVMContext& context) -> llvm::FunctionType*
                 {
                   return llvm::FunctionType::get(
                       /*Result=*/llvm::Type::getVoidTy(context),
                       /*Params=*/ {llvm::Type::getDoubleTy(context)},
                       /*IsVarArg=*/false);
                 },
                 .procedure_addr = std::uint64_t(builtin::put_f64_impl)},
};
}  // namespace

namespace
{
auto type_impl(llvm::LLVMContext& context, ts::void_ const& /*void*/) -> llvm::Type*
{
  return llvm::Type::getVoidTy(context);
}

auto type_impl(llvm::LLVMContext& context, ts::boolean const& /*boolean*/) -> llvm::IntegerType*
{
  return llvm::Type::getInt1Ty(context);
}

auto type_impl(llvm::LLVMContext& context, ts::uint const& unsigned_int) -> llvm::IntegerType*
{
  return llvm::Type::getIntNTy(context, unsigned_int.width);
}

auto type_impl(llvm::LLVMContext& context, ts::sint const& signed_int) -> llvm::IntegerType*
{
  return llvm::Type::getIntNTy(context, signed_int.width);
}

auto type_impl(llvm::LLVMContext& context, ts::single_fp const& /*f32*/) -> llvm::Type*
{
  return llvm::Type::getFloatTy(context);
}

auto type_impl(llvm::LLVMContext& context, ts::double_fp const& /*f32*/) -> llvm::Type*
{
  return llvm::Type::getDoubleTy(context);
}

auto type_impl(llvm::LLVMContext& context, ts::function_signature const& func)
    -> llvm::FunctionType*
{
  auto params = std::vector<llvm::Type*> {};
  for (auto&& fparam : func.parameters) {
    params.emplace_back(bython::backend::type(context, *fparam));
  }

  auto rettype = bython::backend::type(context, *func.rettype);
  return llvm::FunctionType::get(rettype, params, /*isVarArg=*/false);
}
}  // namespace

namespace bython::backend
{

auto type(llvm::LLVMContext& context, ts::type const& type) -> llvm::Type*
{
  switch (type.tag()) {
    // TODO: Improve error handling
    case ts::type_tag::void_:
      return type_impl(context, dynamic_cast<ts::void_ const&>(type));

    case ts::type_tag::boolean:
      return type_impl(context, dynamic_cast<ts::boolean const&>(type));

    case ts::type_tag::uint:
      return type_impl(context, dynamic_cast<ts::uint const&>(type));

    case ts::type_tag::sint:
      return type_impl(context, dynamic_cast<ts::sint const&>(type));

    case ts::type_tag::single_fp:
      return type_impl(context, dynamic_cast<ts::single_fp const&>(type));

    case ts::type_tag::double_fp:
      return type_impl(context, dynamic_cast<ts::double_fp const&>(type));

    case ts::type_tag::function:
      return type_impl(context, dynamic_cast<ts::function_signature const&>(type));
  }
}

auto builtin_function(llvm::LLVMContext& context, ts::function_tag btag) -> builtin_metadata
{
  auto entry = builtin_lookup[static_cast<std::underlying_type_t<ts::function_tag>>(btag)];
  return builtin_metadata {.name = entry.name,
                           .signature = entry.factory(context),
                           .procedure_addr = entry.procedure_addr};
}
}  // namespace bython::backend
