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
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
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
#include "bython/frontend/frontend.hpp"
#include "bython/matching.hpp"
#include "bython/type_system/builtin.hpp"
#include "bython/type_system/environment.hpp"
#include "intrinsic.hpp"
#include "stack.hpp"
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
  explicit codegen_visitor(llvm::Module& out_module, parser::parse_metadata const& metadata_)
      : context {out_module.getContext()}
      , builder {out_module.getContext()}
      , module_ {out_module}
      , metadata {metadata_}
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
    auto ts_function_type = this->environment.add_new_function_type(fdef.sig);
    if (!ts_function_type) {
      log_and_throw("Unable to convert type system repr to LLVM backend");
    }

    auto function_type = ts_function_type.value();
    this->environment.add_new_symbol(fdef.sig.name, function_type);

    auto llvm_function_type =
        llvm::cast<llvm::FunctionType>(backend::definition(this->context, *function_type));
    auto function = llvm::Function::Create(llvm_function_type,
                                           llvm::GlobalValue::LinkageTypes::ExternalLinkage,
                                           fdef.sig.name,
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
    auto storage = this->stack.get(var.identifier);
    if (!storage) {
      this->metadata.report_error(
          var,
          parser::frontend_error_report {.message =
                                             "Failed to find storage on stack for this variable"});
    }

    auto var_type = this->environment.lookup_symbol(var.identifier);
    if (!var_type) {
      this->metadata.report_error(
          var, parser::frontend_error_report {.message = "Failed to find type of this variable"});
    }
    auto llvm_type = backend::definition(this->context, *var_type.value());

    auto* load = this->builder.CreateLoad(llvm_type, *storage, var.identifier);
    return load;
  }

  BYTHON_VISITOR_IMPL(signed_integer, instance)
  {
    auto integer_type = this->environment.get_type(instance);
    if (!integer_type) {
      log_and_throw("Unknown type for signed integer");
    }

    auto llvm_type = backend::definition(this->context, *integer_type.value());
    return llvm::ConstantInt::getSigned(llvm_type, instance.value);
  }

  BYTHON_VISITOR_IMPL(unsigned_integer, instance)
  {
    auto integer_type = this->environment.get_type(instance);
    if (!integer_type) {
      log_and_throw("Unknown type for unsigned integer");
    }

    auto llvm_type = backend::definition(this->context, *integer_type.value());
    return llvm::ConstantInt::get(llvm_type, instance.value, /*IsSigned=*/false);
  }

  BYTHON_VISITOR_IMPL(let_assignment, assgn)
  {
    // Compute RHS of assignment
    auto* rhs_value = this->visit(*assgn.rhs);

    // Load type for RHS
    auto rhs_type = this->environment.get_type(*assgn.rhs);
    if (!rhs_type) {
      this->metadata.report_error(
          *assgn.rhs, parser::frontend_error_report {.message = "Unable to infer for RHS"});
    }

    // Load type for LHS
    auto lhs_type = this->environment.lookup_type(assgn.hint);
    if (!lhs_type) {
      log_and_throw("Unknown type", assgn.hint, "used on LHS of assignment");
    }

    auto subtyped_rhs = this->subtype(*assgn.rhs, rhs_value, *rhs_type, *lhs_type);
    auto allocation =
        this->builder.CreateAlloca(subtyped_rhs->getType(), /*ArraySize=*/nullptr, assgn.lhs);

    this->environment.add_new_symbol(assgn.lhs, lhs_type.value());
    this->stack.put(assgn.lhs, allocation);

    return this->builder.CreateStore(subtyped_rhs, allocation);
  }

  BYTHON_VISITOR_IMPL(binary_operation, binop)
  {
    auto lhs_v = this->visit(*binop.lhs);
    auto lhs_type = this->environment.get_type(*binop.lhs);
    if (!lhs_type) {
      log_and_throw("unknown lhs type");
    }

    llvm::Value* rhs_v = nullptr;
    std::optional<ts::type*> rhs_type = std::nullopt;

    if (binop.op.op != ast::binop_tag::as) {
      // Visit and store type of RHS before calling "real" binary operations
      rhs_v = this->visit(*binop.rhs);
      rhs_type = this->environment.get_type(*binop.rhs);
    }

    switch (binop.op.op) {
      case ast::binop_tag::as: {
        auto target = ast::dyn_cast<ast::variable>(*binop.rhs);
        if (target == nullptr) {
          log_and_throw("`as` expression requires type identifier on RHS");
        }

        rhs_type = this->environment.lookup_type(target->identifier);
        if (!rhs_type) {
          log_and_throw("unknown rhs type");
        }

        return this->subtype(binop, lhs_v, lhs_type.value(), rhs_type.value());
      }

      case ast::binop_tag::pow: {
        // https://llvm.org/docs/LangRef.html#llvm-powi-intrinsic
        // requires powi intrinsics to be brought into scope as prototypes
        auto pow_intrinsic =
            this->insert_or_retrieve_intrinsic(backend::intrinsic_tag::powi_f32_i32);
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
        auto b = this->environment.lookup_type("bool").value();
        lhs_v = this->subtype(*binop.lhs, lhs_v, lhs_type.value(), b);
        rhs_v = this->subtype(*binop.rhs, rhs_v, rhs_type.value(), b);

        return binop.op.op == ast::binop_tag::booland
            ? this->builder.CreateLogicalAnd(lhs_v, rhs_v, "bool.and")
            : this->builder.CreateLogicalOr(lhs_v, rhs_v, "bool.or");
      }
    }
  }

  BYTHON_VISITOR_IMPL(call, instance)
  {
    auto rettype = this->environment.get_type(instance);
    if (!rettype) {
      log_and_throw("Failed to infer type for call");
    }

    auto ft = this->environment.lookup_symbol(instance.callee);
    if (!ft) {
      log_and_throw("Failed to infer type for call");
    }

    auto ft_real = dynamic_cast<ts::func_sig*>(ft.value());
    if (ft_real == nullptr) {
      log_and_throw("Failed to infer type for call");
    }

    if (ft_real->parameters.size() != instance.arguments.arguments.size()) {
      log_and_throw("Wrong amount of arguments");
    }

    auto load_arguments = std::vector<llvm::Value*> {};

    for (decltype(instance.arguments.arguments)::size_type i = 0;
         i < instance.arguments.arguments.size();
         ++i)
    {
      auto&& argument = instance.arguments.arguments[i];
      auto loaded = this->visit(*argument);

      auto argument_type = this->environment.get_type(*argument);
      if (!argument_type) {
        log_and_throw("Unable to infer type of parameter");
      }

      auto subtyped =
          this->subtype(*argument, loaded, argument_type.value(), ft_real->parameters[i]);
      load_arguments.emplace_back(subtyped);
    }

    if (auto defined = this->module_.getFunction(instance.callee); defined != nullptr) {
      return this->builder.CreateCall(defined, load_arguments);
    }
    if (auto intrinsic = this->insert_or_retrieve_intrinsic(instance.callee); intrinsic) {
      return this->builder.CreateCall(*intrinsic, load_arguments);
    }
    if (auto builtin = this->insert_or_retrieve_builtin("bython." + instance.callee)) {
      return this->builder.CreateCall(*builtin, load_arguments);
    }
    log_and_throw("Cannot call function; undefined: ", instance.callee);
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
    // Initialise tables in order of comparison operator tags

    static constexpr auto uint_comp_table = std::array<llvm::CmpInst::Predicate, 6> {
        llvm::CmpInst::Predicate::ICMP_ULT,
        llvm::CmpInst::Predicate::ICMP_ULE,
        llvm::CmpInst::Predicate::ICMP_UGE,
        llvm::CmpInst::Predicate::ICMP_UGT,
        llvm::CmpInst::Predicate::ICMP_EQ,
        llvm::CmpInst::Predicate::ICMP_NE,
    };

    static constexpr auto sint_comp_table = std::array<llvm::CmpInst::Predicate, 6> {
        llvm::CmpInst::Predicate::ICMP_SLT,
        llvm::CmpInst::Predicate::ICMP_SLE,
        llvm::CmpInst::Predicate::ICMP_SGE,
        llvm::CmpInst::Predicate::ICMP_SGT,
        llvm::CmpInst::Predicate::ICMP_EQ,
        llvm::CmpInst::Predicate::ICMP_NE,
    };

    static constexpr auto fp_comp_table = std::array<llvm::CmpInst::Predicate, 6> {
        llvm::CmpInst::Predicate::FCMP_ULT,
        llvm::CmpInst::Predicate::FCMP_ULE,
        llvm::CmpInst::Predicate::FCMP_UGE,
        llvm::CmpInst::Predicate::FCMP_UGT,
        llvm::CmpInst::Predicate::FCMP_UEQ,
        llvm::CmpInst::Predicate::FCMP_UNE,
    };

    // TODO: implement promotion of operands
    auto lhs = this->visit(*instance.lhs);
    auto rhs = this->visit(*instance.rhs);

    auto lhs_t = this->environment.get_type(*instance.lhs);
    if (!lhs_t) {
      log_and_throw("comp lhs infer failed");
    }

    auto rhs_t = this->environment.get_type(*instance.rhs);
    if (!rhs_t) {
      log_and_throw("comp rhs infer failed");
    }

    using cmp_idx = std::underlying_type_t<ast::comparison_operator_tag>;
    if (lhs_t.value()->tag() == ts::type_tag::sint && rhs_t.value()->tag() == ts::type_tag::sint) {
      auto predicate = sint_comp_table[static_cast<cmp_idx>(instance.op.op)];
      return builder.CreateICmp(predicate, lhs, rhs);
    }

    if (lhs_t.value()->tag() == ts::type_tag::uint && rhs_t.value()->tag() == ts::type_tag::uint) {
      auto predicate = uint_comp_table[static_cast<cmp_idx>(instance.op.op)];
      return builder.CreateICmp(predicate, lhs, rhs);
    }

    if (lhs_t.value()->tag() == ts::type_tag::single_fp
        && rhs_t.value()->tag() == ts::type_tag::single_fp)
    {
      auto predicate = fp_comp_table[static_cast<cmp_idx>(instance.op.op)];
      return builder.CreateFCmp(predicate, lhs, rhs);
    }

    if (lhs_t.value()->tag() == ts::type_tag::double_fp
        && rhs_t.value()->tag() == ts::type_tag::double_fp)
    {
      auto predicate = fp_comp_table[static_cast<cmp_idx>(instance.op.op)];
      return builder.CreateFCmp(predicate, lhs, rhs);
    }

    log_and_throw("Failed to codegen floating point operation");
  }

  BYTHON_VISITOR_IMPL(node, instance)
  {
    this->metadata.report_error(instance,
                                parser::frontend_error_report {
                                    .message = "Cannot perform LLVM codegen; Unknown AST Node: "});
    return nullptr;
  }

private:
  auto insert_or_retrieve_intrinsic(backend::intrinsic_tag itag) -> llvm::FunctionCallee
  {
    auto imetadata = backend::intrinsic(this->context, itag);
    auto ir_intrinsic = this->module_.getOrInsertFunction(imetadata.name, imetadata.signature);

    return ir_intrinsic;
  }

  auto insert_or_retrieve_intrinsic(std::string_view name) -> std::optional<llvm::FunctionCallee>
  {
    auto imetadata = backend::intrinsic(this->context, name);
    if (!imetadata) {
      return std::nullopt;
    }

    auto ir_intrinsic = this->module_.getOrInsertFunction(imetadata->name, imetadata->signature);
    return ir_intrinsic;
  }

  auto insert_or_retrieve_builtin(backend::builtin_tag btag) -> llvm::FunctionCallee
  {
    auto bmetadata = backend::builtin(this->context, btag);
    auto ir_intrinsic = this->module_.getOrInsertFunction(bmetadata.name, bmetadata.signature);

    return ir_intrinsic;
  }

  auto insert_or_retrieve_builtin(std::string_view name) -> std::optional<llvm::FunctionCallee>
  {
    auto bmetadata = backend::builtin(this->context, name);
    if (!bmetadata) {
      return std::nullopt;
    }

    auto ir_intrinsic = this->module_.getOrInsertFunction(bmetadata->name, bmetadata->signature);
    return ir_intrinsic;
  }

  auto subtype(ast::node const& node,
               llvm::Value* source_value,
               ts::type* source_type,
               ts::type* target_type) -> llvm::Value*
  {
    auto subtyping_rule = this->environment.try_subtype(*source_type, *target_type);
    if (!subtyping_rule) {
      this->metadata.report_error(
          node, parser::frontend_error_report {.message = "Invalid conversion here!"});
    }

    auto subtype_mapper = backend::subtype_conversion(subtyping_rule.value());
    return subtype_mapper(this->builder, source_value, backend::definition(context, *target_type));
  }

  llvm::LLVMContext& context;
  llvm::IRBuilder<> builder;
  llvm::Module& module_;

  parser::parse_metadata const& metadata;
  type_system::environment environment;
  backend::stack stack;

};  // namespace bython

}  // namespace bython

namespace bython::backend
{
auto compile(std::string_view name,
             std::unique_ptr<node> ast,
             parser::parse_metadata const& metadata,
             llvm::LLVMContext& context) -> std::unique_ptr<llvm::Module>
{
  auto module_ = std::make_unique<llvm::Module>(name, context);
  compile(std::move(ast), metadata, *module_);

  return module_;
}

auto compile(std::unique_ptr<node> ast,
             parser::parse_metadata const& metadata,
             llvm::Module& module_) -> void
{
  auto visitor = codegen_visitor {module_, metadata};
  visitor.visit(*ast);

  llvm::verifyModule(module_, &llvm::errs());
}
}  // namespace bython::backend