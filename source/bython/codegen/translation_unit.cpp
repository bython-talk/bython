#include <map>
#include <tuple>

#include "translation_unit.hpp"

#include <bython/ast.hpp>
#include <bython/matching.hpp>
#include <bython/visitation/ast.hpp>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include "symbols.hpp"
#include "translation_unit.hpp"

using namespace bython::ast;

namespace
{
namespace a = bython::ast;
namespace m = bython::matching;
namespace c = bython::codegen;

struct codegen_visitor final : visitor<codegen_visitor, llvm::Value*>
{
  explicit codegen_visitor(std::string_view module_identifier,
                           llvm::LLVMContext& context_)
      : context {context_}
      , builder {context_}
      , module_ {std::make_unique<llvm::Module>(module_identifier, context_)}
      , symbol_mapping {}
      , type_mapping {}
  {
  }

  BYTHON_VISITOR_IMPL(variable, var)
  {
    auto lookup = this->symbol_mapping.get(var.identifier);
    auto [type, mem_loc] = *lookup;

    auto* load = this->builder.CreateLoad(type, mem_loc, var.identifier);
    return load;
  }

  BYTHON_VISITOR_IMPL(assignment, assgn)
  {
    auto* rhs_value = this->visit(*assgn.rhs);

    if (auto existing_var = this->symbol_mapping.get(assgn.lhs)) {
      auto [type, memory_location] = *existing_var;
      return this->builder.CreateStore(rhs_value, memory_location);
    }

    // TODO: Enforce type declarations for assignments
    //  auto type = this->type_mapping.get(assgn.hint);
    auto* type = llvm::IntegerType::get(this->context, 64);
    auto allocation =
        this->builder.CreateAlloca(type, /*ArraySize=*/nullptr, assgn.lhs);
    this->symbol_mapping.put(assgn.lhs, type, allocation);
    return this->builder.CreateStore(rhs_value, allocation);
  }

  BYTHON_VISITOR_IMPL(binary_operation, binop)
  {
    auto lhs_v = this->visit(*binop.lhs);
    auto rhs_v = this->visit(*binop.rhs);

    if (m::matches(*binop.op, m::binary_operator {a::binop_tag::plus})) {
      return this->builder.CreateAdd(lhs_v, rhs_v, "plus");
    }
    return this->builder.CreateSub(lhs_v, rhs_v, "minus");
  }

  BYTHON_VISITOR_IMPL(node, /*instance*/)
  {
    throw std::runtime_error {"Unknown AST Node"};
  }

  llvm::LLVMContext& context;
  llvm::IRBuilder<> builder;
  std::unique_ptr<llvm::Module> module_;

  c::symbol_lookup symbol_mapping;
  c::type_lookup type_mapping;
};

}  // namespace

namespace bython::codegen
{

auto translation_unit::into_module() const -> std::unique_ptr<llvm::Module>
{
  auto context = llvm::LLVMContext();
  auto visitor = codegen_visitor {"module", context};

  for (auto&& stmt : this->module_.body) {
    visitor.visit(*stmt);
  }
  return std::move(visitor.module_);
}
}  // namespace bython::codegen