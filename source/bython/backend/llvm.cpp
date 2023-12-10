#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <tuple>
#include <type_traits>

#include "llvm.hpp"

#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Triple.h>

#include "builtin.hpp"
#include "bython/ast.hpp"
#include "bython/ast/operators.hpp"
#include "bython/ast/statement.hpp"
#include "bython/ast/visitor.hpp"
#include "bython/matching.hpp"
#include "bython/type_system/builtin.hpp"
#include "bython/type_system/environment.hpp"
#include "intrinsic.hpp"
#include "symbols.hpp"
#include "typing.hpp"

namespace bython
{

using namespace bython::ast;  // This line is required for the visitor macros to function properly
// namespace ast = bython::ast;
namespace m = bython::matching;
namespace ts = bython::type_system;

template<typename Arg, typename... Args>
[[noreturn]] auto log_and_throw(Arg&& arg, Args&&... args) -> void
{
  std::string error;
  auto rso = llvm::raw_string_ostream {error};

  rso << std::forward<Arg>(arg);
  ((rso << ' ' << std::forward<Args>(args)), ...);
  llvm::errs() << error << '\n';

  throw std::logic_error {std::move(error)};
}

struct codegen_visitor final : visitor<codegen_visitor, llvm::Value*>
{
  explicit codegen_visitor(llvm::Module& out_module)
      : context {out_module.getContext()}
      , builder {out_module.getContext()}
      , module_ {out_module}
      , environment {ts::environment::initialise_with_builtins()}
  {
  }

  BYTHON_VISITOR_IMPL(mod, m)
  {
    for (auto&& stmt : m.body) {
      this->visit(*stmt);
    }
    return nullptr;
  }

  BYTHON_VISITOR_IMPL(function_def, fdef)
  {
    auto ts_function_type = this->environment.add_new_function_type(fdef);
    if (!ts_function_type) {
      log_and_throw("Unable to convert type system repr to LLVM backend");
    }

    auto function_type = ts_function_type.value();
    this->environment.add_new_symbol(fdef.name, function_type);

    auto llvm_function_type =
        llvm::cast<llvm::FunctionType>(codegen::definition(this->context, *function_type));
    auto function = llvm::Function::Create(llvm_function_type,
                                           llvm::GlobalValue::LinkageTypes::ExternalLinkage,
                                           fdef.name,
                                           this->module_);

    auto entry_into_function = llvm::BasicBlock::Create(this->context, "entry", function);
    this->builder.SetInsertPoint(entry_into_function);

    for (auto&& stmt : fdef.body) {
      this->visit(*stmt);
    }

    // Support implicit return
    if (llvm_function_type->getReturnType()->isVoidTy()) {
      auto& last_basicblock = function->back();
      if (!last_basicblock.empty() && !llvm::isa<llvm::ReturnInst>(last_basicblock.back())) {
        this->builder.CreateRetVoid();
      }
    }

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

  BYTHON_VISITOR_IMPL(signed_integer, instance)
  {
    auto integer_type = this->environment.infer(instance);
    if (!integer_type) {
      log_and_throw("Unknown type for signed integer");
    }

    auto llvm_type = codegen::definition(this->context, *integer_type.value());
    return llvm::ConstantInt::getSigned(llvm_type, instance.value);
  }

  BYTHON_VISITOR_IMPL(unsigned_integer, instance)
  {
    auto integer_type = this->environment.infer(instance);
    if (!integer_type) {
      log_and_throw("Unknown type for unsigned integer");
    }

    auto llvm_type = codegen::definition(this->context, *integer_type.value());
    return llvm::ConstantInt::get(llvm_type, instance.value, /*IsSigned=*/false);
  }

  BYTHON_VISITOR_IMPL(assignment, assgn)
  {
    // Compute RHS of assignment
    auto* rhs_value = this->visit(*assgn.rhs);

    // Load type for LHS
    auto lhs_type = this->environment.lookup(assgn.hint);
    if (!lhs_type) {
      log_and_throw("Unknown type", assgn.hint, "used on LHS of assignment");
    }

    // Load type for RHS
    auto rhs_type = this->environment.infer(*assgn.rhs);
    if (!rhs_type) {
      log_and_throw("Unable to infer for RHS");
    }

    // Account for subtyping
    auto subtype_rule = this->environment.try_subtype(*lhs_type.value(), *rhs_type.value());
    if (!subtype_rule) {
      log_and_throw("Unknown RHS type");
    }

    auto subtyper_impl = codegen::subtype_conversion(*subtype_rule);

    auto lhs_llvm_type = codegen::definition(this->context, *lhs_type.value());
    auto subtyped = subtyper_impl(this->builder, rhs_value, lhs_llvm_type);

    // Load type for RHS
    if (auto existing_var = this->symbol_mapping.get(assgn.lhs)) {
      auto [type, memory_location] = *existing_var;
      return this->builder.CreateStore(rhs_value, memory_location);
    }

    auto allocation = this->builder.CreateAlloca(lhs_llvm_type, /*ArraySize=*/nullptr, assgn.lhs);
    this->symbol_mapping.put(assgn.lhs, lhs_llvm_type, allocation);

    return this->builder.CreateStore(subtyped, allocation);
  }

  BYTHON_VISITOR_IMPL(binary_operation, binop)
  {
    auto lhs_v = this->visit(*binop.lhs);
    auto lhs_type = this->environment.infer(*binop.lhs);
    if (!lhs_type) {
      log_and_throw("unknown lhs type");
    }

    llvm::Value* rhs_v = nullptr;
    std::optional<ts::type*> rhs_type = std::nullopt;

    if (binop.op.op != ast::binop_tag::as) {
      // Visit and store type of RHS before calling "real" binary operations
      rhs_v = this->visit(*binop.rhs);
      rhs_type = this->environment.infer(*binop.rhs);
    }

    switch (binop.op.op) {
      case ast::binop_tag::as: {
        auto target = ast::dyn_cast<ast::variable>(*binop.rhs);
        if (target == nullptr) {
          log_and_throw("`as` expression requires type identifier on RHS");
        }

        rhs_type = this->environment.lookup(target->identifier);
        if (!rhs_type) {
          log_and_throw("unknown rhs type");
        }

        auto subtype_converter =
            this->environment.try_subtype(*lhs_type.value(), *rhs_type.value());
        if (!subtype_converter) {
          log_and_throw("subtype conversion failed");
        }

        auto subtyping_rule = subtype_converter.value();
        auto subtyper_impl = codegen::subtype_conversion(subtyping_rule);
        return subtyper_impl(this->builder, lhs_v, codegen::definition(context, *rhs_type.value()));
      }

      case ast::binop_tag::pow: {
        // https://llvm.org/docs/LangRef.html#llvm-powi-intrinsic
        // requires powi intrinsics to be brought into scope as prototypes
        auto pow_intrinsic =
            this->insert_or_retrieve_intrinsic(codegen::intrinsic_tag::powi_f32_i32);
        return this->builder.CreateCall(pow_intrinsic, {lhs_v, rhs_v});
      }
      case ast::binop_tag::multiply: {
        return this->builder.CreateMul(lhs_v, rhs_v, "a.mul");
      }
      case ast::binop_tag::divide: {
        return this->builder.CreateSDiv(lhs_v, rhs_v, "a.sdiv");
      }
      case ast::binop_tag::modulo: {
        return this->builder.CreateSRem(lhs_v, rhs_v, "a.srem");
      }
      case ast::binop_tag::plus: {
        return this->builder.CreateAdd(lhs_v, rhs_v, "a.add");
      }
      case ast::binop_tag::minus: {
        return this->builder.CreateSub(lhs_v, rhs_v, "a.sub");
      }
      case ast::binop_tag::bitshift_right_: {
        return this->builder.CreateAShr(lhs_v, rhs_v, "bit.ashr");
      }
      case ast::binop_tag::bitshift_left_: {
        return this->builder.CreateShl(lhs_v, rhs_v, "bit.shl");
      }
      case ast::binop_tag::bitand_: {
        return this->builder.CreateAnd(lhs_v, rhs_v, "bit.and");
      }
      case ast::binop_tag::bitxor_: {
        return this->builder.CreateXor(lhs_v, rhs_v, "bit.xor");
      }
      case ast::binop_tag::bitor_: {
        return this->builder.CreateOr(lhs_v, rhs_v, "bit.or");
      }
      case ast::binop_tag::booland:
      case ast::binop_tag::boolor: {
        auto subtype_converter =
            this->environment.try_subtype(*lhs_type.value(), type_system::boolean {});
        if (!subtype_converter) {
          log_and_throw("subtype conversion failed");
        }

        auto subtyping_rule = subtype_converter.value();
        auto subtyper_impl = codegen::subtype_conversion(subtyping_rule);
        lhs_v =
            subtyper_impl(this->builder, lhs_v, codegen::definition(context, *rhs_type.value()));

        subtype_converter =
            this->environment.try_subtype(*rhs_type.value(), type_system::boolean {});
        if (!subtype_converter) {
          log_and_throw("subtype conversion failed");
        }

        subtyping_rule = subtype_converter.value();
        subtyper_impl = codegen::subtype_conversion(subtyping_rule);
        rhs_v =
            subtyper_impl(this->builder, rhs_v, codegen::definition(context, *rhs_type.value()));

        return binop.op.op == ast::binop_tag::booland
            ? this->builder.CreateLogicalAnd(lhs_v, rhs_v, "bool.and")
            : this->builder.CreateLogicalOr(lhs_v, rhs_v, "bool.or");
      }
    }
  }

  BYTHON_VISITOR_IMPL(call, instance)
  {
    auto load_arguments = std::vector<llvm::Value*> {};
    for (auto&& argument : instance.arguments.arguments) {
      load_arguments.emplace_back(this->visit(*argument));
    }

    if (auto defined = this->module_.getFunction(instance.callee); defined != nullptr) {
      return this->builder.CreateCall(defined, load_arguments);
    } else if (auto intrinsic = this->insert_or_retrieve_intrinsic(instance.callee); intrinsic) {
      return this->builder.CreateCall(*intrinsic, load_arguments);
    } else if (auto builtin = this->insert_or_retrieve_builtin("bython." + instance.callee))
      return this->builder.CreateCall(*builtin, load_arguments);
    else {
      log_and_throw("Cannot call function; undefined: ", instance.callee);
    }
  }

  BYTHON_VISITOR_IMPL(expression_statement, instance)
  {
    this->visit(*instance.discarded);
    return nullptr;
  }

  BYTHON_VISITOR_IMPL(return_, instance)
  {
    auto value = this->visit(*instance.expr);
    return this->builder.CreateRet(value);
  }

  BYTHON_VISITOR_IMPL(conditional_branch, instance)
  {
    auto branch_on = this->builder.CreateICmpNE(
        this->visit(*instance.condition),
        llvm::ConstantInt::get(this->context, llvm::APInt(1, 0, /*isSigned=*/false)));

    auto parent = this->builder.GetInsertBlock()->getParent();
    auto if_true = llvm::BasicBlock::Create(this->context, "iftrue");
    auto otherwise = llvm::BasicBlock::Create(this->context, "otherwise");
    auto joined = llvm::BasicBlock::Create(this->context, "merge");
    parent->insert(parent->end(), if_true);

    this->builder.CreateCondBr(branch_on, if_true, otherwise);

    this->builder.SetInsertPoint(if_true);
    for (auto&& stmt : instance.body) {
      this->visit(*stmt);
    }
    this->builder.CreateBr(joined);

    if (instance.orelse != nullptr) {
      this->visit(*instance.orelse);
    }
    otherwise = this->builder.GetInsertBlock();

    parent->insert(parent->end(), joined);
    this->builder.SetInsertPoint(joined);

    return nullptr;
  }

  BYTHON_VISITOR_IMPL(comparison, instance)
  {
    using namespace std::string_view_literals;

#define BUILDER_ALIAS(BUILDER_FUNCTION) \
  [](llvm::IRBuilder<>& builder_, llvm::Value* lhs_, llvm::Value* rhs_, llvm::Twine const& name_) \
      -> llvm::Value* { return builder_.BUILDER_FUNCTION(lhs_, rhs_, name_); }

    using op_table_lookup = std::array<std::tuple<std::string_view,
                                                  llvm::Value* (*)(llvm::IRBuilder<>& builder,
                                                                   llvm::Value*,
                                                                   llvm::Value*,
                                                                   llvm::Twine const&)>,
                                       6>;

    static constexpr auto signed_integer_op_table =
        op_table_lookup {std::make_tuple("cmp.slt"sv, BUILDER_ALIAS(CreateICmpSLT)),
                         std::make_tuple("cmp.sle"sv, BUILDER_ALIAS(CreateICmpSLE)),
                         std::make_tuple("cmp.sgt"sv, BUILDER_ALIAS(CreateICmpSGE)),
                         std::make_tuple("cmp.sge"sv, BUILDER_ALIAS(CreateICmpSGT)),
                         std::make_tuple("cmp.seq"sv, BUILDER_ALIAS(CreateICmpEQ)),
                         std::make_tuple("cmp.sne"sv, BUILDER_ALIAS(CreateICmpNE))};

#undef BUILDER_ALIAS

    /*static constexpr auto unsigned_integer_op_table =
        op_table_lookup {{comparison_operator_tag::lsr, this->builder.CreateICmpULT},
                         {comparison_operator_tag::leq, this->builder.CreateICmpULE},
                         {comparison_operator_tag::geq, this->builder.CreateICmpUGE},
                         {comparison_operator_tag::grt, this->builder.CreateICmpUGT},
                         {comparison_operator_tag::eq, this->builder.CreateICmpEQ},
                         {comparison_operator_tag::ne, this->builder.CreateICmpNE}};

    static constexpr auto floating_op_table =
        op_table_lookup {{comparison_operator_tag::lsr, this->builder.CreateFCmpOLT},
                         {comparison_operator_tag::leq, this->builder.CreateFCmpOLE},
                         {comparison_operator_tag::geq, this->builder.CreateFCmpOGE},
                         {comparison_operator_tag::grt, this->builder.CreateFCmpOGT},
                         {comparison_operator_tag::eq, this->builder.CreateFCmpOEQ},
                         {comparison_operator_tag::ne, this->builder.CreateFCmpONE}};*/

    // Load all information, prepend boolean true
    auto cs = std::vector<llvm::Value*> {};
    for (auto&& operand : instance.operands) {
      cs.emplace_back(this->visit(*operand));
    }

    // For now only support Integer and Floating types
    // NOTE: the length of cs is instance.operands.size() + 1, so this is safe
    llvm::Value* logical_and_accum =
        llvm::ConstantInt::getSigned(llvm::Type::getInt1Ty(this->context), 1);

    for (auto operand = 0U; operand < instance.operands.size(); ++operand) {
      auto lhs = cs[operand + 0];
      auto rhs = cs[operand + 1];

      // Promote operands to identical types
      if (auto unified = codegen::unify(this->builder, lhs, rhs); unified) {
        lhs = std::get<0>(*unified);
        rhs = std::get<1>(*unified);
      } else {
        log_and_throw(
            "Unable to create comparison between ", *lhs->getType(), " and ", *rhs->getType());
      }

      auto* op = dyn_cast<comparison_operator>(*instance.ops[operand]);
      if (!op) {
        log_and_throw("Unknown operator used in comparison");
      }

      // TODO: Account for unsigned integers
      auto [name, builder_function] =
          signed_integer_op_table[std::underlying_type_t<comparison_operator_tag>(op->op)];
      auto comp_exec = builder_function(builder, lhs, rhs, name);

      auto comp_as_bool =
          codegen::convert(builder, comp_exec, llvm::Type::getInt1Ty(this->context));
      if (!comp_as_bool) {
        log_and_throw("Unable to convert result of comparison to bool");
      }

      logical_and_accum =
          this->builder.CreateLogicalAnd(logical_and_accum,
                                         *comp_as_bool,
                                         std::string {"comp.booland."} + std::to_string(operand));
    }

    return logical_and_accum;
  }

  BYTHON_VISITOR_IMPL(node, instance)
  {
    throw std::runtime_error {"Cannot perform LLVM codegen; Unknown AST Node: "
                              + std::to_string(instance.tag().unwrap())};
  }

private:
  auto insert_or_retrieve_intrinsic(codegen::intrinsic_tag itag) -> llvm::FunctionCallee
  {
    auto imetadata = codegen::intrinsic(this->context, itag);
    auto ir_intrinsic = this->module_.getOrInsertFunction(imetadata.name, imetadata.signature);

    return ir_intrinsic;
  }

  auto insert_or_retrieve_intrinsic(std::string_view name) -> std::optional<llvm::FunctionCallee>
  {
    auto imetadata = codegen::intrinsic(this->context, name);
    if (!imetadata) {
      return std::nullopt;
    }

    auto ir_intrinsic = this->module_.getOrInsertFunction(imetadata->name, imetadata->signature);
    return ir_intrinsic;
  }

  auto insert_or_retrieve_builtin(codegen::builtin_tag btag) -> llvm::FunctionCallee
  {
    auto bmetadata = codegen::builtin(this->context, btag);
    auto ir_intrinsic = this->module_.getOrInsertFunction(bmetadata.name, bmetadata.signature);

    return ir_intrinsic;
  }

  auto insert_or_retrieve_builtin(std::string_view name) -> std::optional<llvm::FunctionCallee>
  {
    auto bmetadata = codegen::builtin(this->context, name);
    if (!bmetadata) {
      return std::nullopt;
    }

    auto ir_intrinsic = this->module_.getOrInsertFunction(bmetadata->name, bmetadata->signature);
    return ir_intrinsic;
  }

  llvm::LLVMContext& context;
  llvm::IRBuilder<> builder;
  llvm::Module& module_;

  codegen::symbol_lookup symbol_mapping;
  codegen::type_lookup type_mapping;

  type_system::environment environment;
};  // namespace bython

}  // namespace bython

namespace bython::codegen
{
auto compile(std::string_view name, std::unique_ptr<node> ast, llvm::LLVMContext& context)
    -> std::unique_ptr<llvm::Module>
{
  auto module_ = std::make_unique<llvm::Module>(name, context);
  compile(std::move(ast), *module_);

  return module_;
}

auto compile(std::unique_ptr<node> ast, llvm::Module& module_) -> void
{
  auto visitor = codegen_visitor {module_};
  visitor.visit(*ast);

  llvm::verifyModule(module_, &llvm::errs());
}
}  // namespace bython::codegen