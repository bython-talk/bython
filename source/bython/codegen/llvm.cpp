#include <map>
#include <tuple>

#include "llvm.hpp"

#include <bython/ast.hpp>
#include <bython/matching.hpp>
#include <bython/visitation/ast.hpp>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>

#include "intrinsic.hpp"
#include "symbols.hpp"

namespace bython
{
using namespace bython::ast;
namespace a = bython::ast;
namespace m = bython::matching;

struct codegen_visitor final : visitor<codegen_visitor, llvm::Value*>
{
  explicit codegen_visitor(std::string_view module_identifier, llvm::LLVMContext& context_)
      : context {context_}
      , builder {context_}
      , module_ {std::make_unique<llvm::Module>(module_identifier, context_)}
      , symbol_mapping {}
      , type_mapping {}
  {
  }

  BYTHON_VISITOR_IMPL(mod, m)
  {
    for (auto&& stmt : m.body) {
      this->visit(*stmt);
    }

    // TODO: This should never be used; read from module attribute instead
    return nullptr;
  }

  BYTHON_VISITOR_IMPL(function_def, fdef)
  {
    auto function_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(this->context), {}, /*isVarArg=*/false);
    auto function = llvm::Function::Create(
        function_type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, fdef.name, *this->module_);

    auto entry_into_function = llvm::BasicBlock::Create(this->context, "entry", function);
    this->builder.SetInsertPoint(entry_into_function);

    for (auto&& stmt : fdef.body) {
      this->visit(*stmt);
    }

    this->builder.CreateRetVoid();
    llvm::verifyFunction(*function);

    return function;
  }

  BYTHON_VISITOR_IMPL(variable, var)
  {
    auto lookup = this->symbol_mapping.get(var.identifier);
    auto [type, mem_loc] = *lookup;

    auto* load = this->builder.CreateLoad(type, mem_loc, var.identifier);
    return load;
  }

  BYTHON_VISITOR_IMPL(integer, instance)
  {
    return llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(this->context), instance.value);
  }

  BYTHON_VISITOR_IMPL(assignment, assgn)
  {
    auto* rhs_value = this->visit(*assgn.rhs);
    if (assgn.lhs == "_") {
      return rhs_value;
    }

    if (auto existing_var = this->symbol_mapping.get(assgn.lhs)) {
      auto [type, memory_location] = *existing_var;
      return this->builder.CreateStore(rhs_value, memory_location);
    }

    // TODO: Enforce type declarations for assignments
    //  auto type = this->type_mapping.get(assgn.hint);
    auto* type = llvm::Type::getInt64Ty(this->context);
    auto allocation = this->builder.CreateAlloca(type, /*ArraySize=*/nullptr, assgn.lhs);
    this->symbol_mapping.put(assgn.lhs, type, allocation);
    return this->builder.CreateStore(rhs_value, allocation);
  }

  BYTHON_VISITOR_IMPL(binary_operation, binop)
  {
    auto lhs_v = this->visit(*binop.lhs);
    auto rhs_v = this->visit(*binop.rhs);

    if (m::matches(*binop.op, m::binary_operator {a::binop_tag::pow})) {
      // https://llvm.org/docs/LangRef.html#llvm-powi-intrinsic
      // requires powi intrinsics to be brought into scope as prototypes
      auto pow_intrinsic = this->insert_or_retrieve_intrinsic(codegen::intrinsic_tag::powi_f32_i32);
      return this->builder.CreateCall(pow_intrinsic, {lhs_v, rhs_v});
    } else if (m::matches(*binop.op, m::binary_operator {a::binop_tag::plus})) {
      return this->builder.CreateAdd(lhs_v, rhs_v, "plus");
    }
    return this->builder.CreateSub(lhs_v, rhs_v, "minus");
  }

  BYTHON_VISITOR_IMPL(call, instance)
  {
    auto load_arguments = std::vector<llvm::Value*> {};
    for (auto&& argument : instance.arguments) {
      load_arguments.emplace_back(this->visit(*argument));
    }

    if (auto callee = this->module_->getFunction(instance.callee); callee != nullptr) {
      return this->builder.CreateCall(callee, load_arguments);
    } else if (instance.callee == "put_i64") {
      auto intrinsic_callee = this->insert_or_retrieve_intrinsic(codegen::intrinsic_tag::put_i64);
      return this->builder.CreateCall(intrinsic_callee, load_arguments);
    } else {
      throw std::logic_error {"Unknown function: " + instance.callee};
    }
  }

  BYTHON_VISITOR_IMPL(node, /*instance*/)
  {
    throw std::runtime_error {"Unknown AST Node"};
  }

private:
  auto insert_or_retrieve_intrinsic(codegen::intrinsic_tag itag) -> llvm::FunctionCallee
  {
    auto intrinsic = codegen::builtin_intrinsic(this->context, itag);
    auto ir_intrinsic = this->module_->getOrInsertFunction(intrinsic.name, intrinsic.signature);

    return ir_intrinsic;
  }

public:
  llvm::LLVMContext& context;
  llvm::IRBuilder<> builder;
  std::unique_ptr<llvm::Module> module_;

  codegen::symbol_lookup symbol_mapping;
  codegen::type_lookup type_mapping;
};

}  // namespace bython

namespace bython::codegen
{
auto compile(std::string_view name,
             std::unique_ptr<ast::node> ast,
             llvm::LLVMContext& context) -> std::unique_ptr<llvm::Module>
{
  auto visitor = codegen_visitor {name, context};
  visitor.visit(*ast);

  return std::move(visitor.module_);
}
}  // namespace bython::codegen