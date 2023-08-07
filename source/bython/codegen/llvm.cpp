#include <map>
#include <tuple>

#include "llvm.hpp"

#include <bython/ast.hpp>
#include <bython/matching.hpp>
#include <bython/visitation/ast.hpp>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/TargetParser/Triple.h>

#include "builtin.hpp"
#include "intrinsic.hpp"
#include "symbols.hpp"
#include "type_system.hpp"

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
    llvm::verifyFunction(*function, &llvm::errs());

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
    if (std::numeric_limits<std::int8_t>::lowest() <= instance.value
        && instance.value <= std::numeric_limits<std::int8_t>::max())
    {
      return llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(this->context), instance.value);
    }

    else if (std::numeric_limits<std::int16_t>::lowest() <= instance.value
             && instance.value <= std::numeric_limits<std::int16_t>::max())
    {
      return llvm::ConstantInt::getSigned(llvm::Type::getInt16Ty(this->context), instance.value);
    }

    else if (std::numeric_limits<std::int32_t>::lowest() <= instance.value
             && instance.value <= std::numeric_limits<std::int32_t>::max())
    {
      return llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(this->context), instance.value);
    }

    else if (std::numeric_limits<std::int64_t>::lowest() <= instance.value
             && instance.value <= std::numeric_limits<std::int64_t>::max())
    {
      return llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(this->context), instance.value);
    }

    else
    {
      throw std::logic_error {"Integer constant " + std::to_string(instance.value)
                              + " is too large; does not fit in widest integer datatype (i64)"};
    }
  }

  BYTHON_VISITOR_IMPL(assignment, assgn)
  {
    auto* rhs_value = this->visit(*assgn.rhs);
    if (auto existing_var = this->symbol_mapping.get(assgn.lhs)) {
      auto [type, memory_location] = *existing_var;
      return this->builder.CreateStore(rhs_value, memory_location);
    }

    auto lhs_type = this->type_mapping.get(this->context, assgn.hint);
    if (!lhs_type) {
      throw std::logic_error {"Unknown type used in LHS of assignment: " + assgn.hint};
    }

    auto converted_rhs = codegen::convert(this->builder, rhs_value, *lhs_type);
    if (!converted_rhs) {
      throw std::logic_error {"Unable to convert RHS to type of LHS: " + assgn.hint};
    }

    auto allocation = this->builder.CreateAlloca(*lhs_type, /*ArraySize=*/nullptr, assgn.lhs);
    this->symbol_mapping.put(assgn.lhs, *lhs_type, allocation);
    return this->builder.CreateStore(*converted_rhs, allocation);
  }

  BYTHON_VISITOR_IMPL(binary_operation, binop)
  {
    if (m::matches(*binop.op, m::binary_operator {a::binop_tag::as})) {
      auto lhs_v = this->visit(*binop.lhs);
      auto target = a::dyn_cast<ast::variable>(*binop.rhs);

      if (target == nullptr) {
        throw std::logic_error {"`as` expression requires type identifier on RHS"};
      }

      auto target_type = this->type_mapping.get(this->context, target->identifier);
      if (!target_type) {
        throw std::logic_error {"Unknown type on RHS: " + target->identifier};
      }

      auto conversion = codegen::convert(this->builder, lhs_v, *target_type);
      if (!conversion) {
        throw std::logic_error {"Unable to codegen conversion"};
      }
      return *conversion;
    }

    else
    {
      auto lhs_v = this->visit(*binop.lhs);
      auto rhs_v = this->visit(*binop.rhs);

      if (auto* b = ast::dyn_cast<ast::binary_operator>(*binop.op); b) {
        switch (b->op) {
          case a::binop_tag::pow: {
            // https://llvm.org/docs/LangRef.html#llvm-powi-intrinsic
            // requires powi intrinsics to be brought into scope as prototypes
            auto pow_intrinsic =
                this->insert_or_retrieve_intrinsic(codegen::intrinsic_tag::powi_f32_i32);
            return this->builder.CreateCall(pow_intrinsic, {lhs_v, rhs_v});
          }
          case a::binop_tag::multiply: {
            return this->builder.CreateMul(lhs_v, rhs_v, "a.mul");
          }
          case a::binop_tag::divide: {
            return this->builder.CreateSDiv(lhs_v, rhs_v, "a.sdiv");
          }
          case a::binop_tag::modulo: {
            return this->builder.CreateSRem(lhs_v, rhs_v, "a.srem");
          }
          case a::binop_tag::plus: {
            return this->builder.CreateAdd(lhs_v, rhs_v, "a.add");
          }
          case a::binop_tag::minus: {
            return this->builder.CreateSub(lhs_v, rhs_v, "a.sub");
          }
          case a::binop_tag::bitshift_right_: {
            return this->builder.CreateAShr(lhs_v, rhs_v, "bit.ashr");
          }
          case a::binop_tag::bitshift_left_: {
            return this->builder.CreateShl(lhs_v, rhs_v, "bit.shl");
          }
          case a::binop_tag::bitand_: {
            return this->builder.CreateAnd(lhs_v, rhs_v, "bit.and");
          }
          case a::binop_tag::bitxor_: {
            return this->builder.CreateXor(lhs_v, rhs_v, "bit.xor");
          }
          case a::binop_tag::bitor_: {
            return this->builder.CreateOr(lhs_v, rhs_v, "bit.or");
          }
          case a::binop_tag::booland: {
            auto lhs_bool =
                codegen::convert(this->builder, lhs_v, llvm::Type::getInt1Ty(this->context));
            if (!lhs_bool) {
              throw std::logic_error {"Unable to convert LHS to i1 (boolean)"};
            }
            auto rhs_bool =
                codegen::convert(this->builder, rhs_v, llvm::Type::getInt1Ty(this->context));
            if (!rhs_bool) {
              throw std::logic_error {"Unable to convert RHS to i1 (boolean)"};
            }

            return this->builder.CreateLogicalAnd(*lhs_bool, *rhs_bool, "log.and");
          }
          case a::binop_tag::boolor: {
            auto lhs_bool =
                codegen::convert(this->builder, lhs_v, llvm::Type::getInt1Ty(this->context));
            if (!lhs_bool) {
              throw std::logic_error {"Unable to convert LHS to i1 (boolean)"};
            }
            auto rhs_bool =
                codegen::convert(this->builder, rhs_v, llvm::Type::getInt1Ty(this->context));
            if (!rhs_bool) {
              throw std::logic_error {"Unable to convert RHS to i1 (boolean)"};
            }

            return this->builder.CreateLogicalOr(*lhs_bool, *rhs_bool, "log.or");
          }
        }
      }
    }

    llvm_unreachable("Failed to handle all binary operations");
  }

  BYTHON_VISITOR_IMPL(call, instance)
  {
    auto load_arguments = std::vector<llvm::Value*> {};
    for (auto&& argument : instance.arguments) {
      load_arguments.emplace_back(this->visit(*argument));
    }

    if (auto defined = this->module_->getFunction(instance.callee); defined != nullptr) {
      return this->builder.CreateCall(defined, load_arguments);
    } else if (auto intrinsic = this->insert_or_retrieve_intrinsic(instance.callee); intrinsic) {
      return this->builder.CreateCall(*intrinsic, load_arguments);
    } else if (auto builtin = this->insert_or_retrieve_builtin("bython." + instance.callee))
      return this->builder.CreateCall(*builtin, load_arguments);
    else {
      throw std::logic_error {"Cannot call function; undefined: " + instance.callee};
    }
  }

  BYTHON_VISITOR_IMPL(expression_statement, instance)
  {
    this->visit(*instance.discarded);
    return nullptr;
  }

  BYTHON_VISITOR_IMPL(node, /*instance*/)
  {
    throw std::runtime_error {"Unknown AST Node"};
  }

private:
  auto insert_or_retrieve_intrinsic(codegen::intrinsic_tag itag) -> llvm::FunctionCallee
  {
    auto imetadata = codegen::intrinsic(this->context, itag);
    auto ir_intrinsic = this->module_->getOrInsertFunction(imetadata.name, imetadata.signature);

    return ir_intrinsic;
  }

  auto insert_or_retrieve_intrinsic(std::string_view name) -> std::optional<llvm::FunctionCallee>
  {
    auto imetadata = codegen::intrinsic(this->context, name);
    if (!imetadata) {
      return std::nullopt;
    }

    auto ir_intrinsic = this->module_->getOrInsertFunction(imetadata->name, imetadata->signature);
    return ir_intrinsic;
  }

  auto insert_or_retrieve_builtin(codegen::builtin_tag btag) -> llvm::FunctionCallee
  {
    auto bmetadata = codegen::builtin(this->context, btag);
    auto ir_intrinsic = this->module_->getOrInsertFunction(bmetadata.name, bmetadata.signature);

    return ir_intrinsic;
  }

  auto insert_or_retrieve_builtin(std::string_view name) -> std::optional<llvm::FunctionCallee>
  {
    auto bmetadata = codegen::builtin(this->context, name);
    if (!bmetadata) {
      return std::nullopt;
    }

    auto ir_intrinsic = this->module_->getOrInsertFunction(bmetadata->name, bmetadata->signature);
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
auto compile(std::string_view name, std::unique_ptr<ast::node> ast, llvm::LLVMContext& context)
    -> std::unique_ptr<llvm::Module>
{
  auto visitor = codegen_visitor {name, context};
  visitor.visit(*ast);

  llvm::verifyModule(*visitor.module_, &llvm::errs());
  return std::move(visitor.module_);
}
}  // namespace bython::codegen