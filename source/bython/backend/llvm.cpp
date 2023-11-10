#include <iomanip>
#include <map>
#include <sstream>
#include <tuple>
#include <type_traits>

#include "llvm.hpp"

#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Triple.h>

#include "builtin.hpp"
#include "bython/ast.hpp"
#include "bython/ast/visitor.hpp"
#include "bython/matching.hpp"
#include "intrinsic.hpp"
#include "symbols.hpp"
#include "type_system.hpp"

namespace bython
{
using namespace bython::ast;
namespace a = bython::ast;
namespace m = bython::matching;

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
      , symbol_mapping {}
      , type_mapping {}
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
    auto parameter_types = std::vector<llvm::Type*> {};
    for (auto&& parameter : fdef.parameters.parameters) {
      if (auto retrieved = this->type_mapping.get(this->context, parameter.hint); retrieved) {
        parameter_types.emplace_back(retrieved->type);
      } else {
        log_and_throw("Failed to codegen parameters for '",
                      fdef.name,
                      "'; undefined type on parameter '",
                      parameter.name,
                      "' - '",
                      parameter.hint,
                      "'");
      }
    }

    auto function_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(this->context), {}, /*isVarArg=*/false);
    auto function = llvm::Function::Create(
        function_type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, fdef.name, this->module_);

    auto entry_into_function = llvm::BasicBlock::Create(this->context, "entry", function);
    this->builder.SetInsertPoint(entry_into_function);

    for (auto&& stmt : fdef.body) {
      this->visit(*stmt);
    }

    // Support implicit return
    if (function_type->getReturnType()->isVoidTy()) {
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

  BYTHON_VISITOR_IMPL(integer, instance)
  {
    if (std::numeric_limits<std::int64_t>::lowest() <= instance.value
             && instance.value <= std::numeric_limits<std::int64_t>::max())
    {
      return llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(this->context), instance.value);
    }

    else
    {
      log_and_throw("Integer constant",
                    std::to_string(instance.value),
                    "does not fit in any integer datatype");
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
      log_and_throw("Unknown type", assgn.hint, "used on LHS of assignment");
    }

    auto converted_rhs = codegen::convert(this->builder, rhs_value, lhs_type->type);
    if (!converted_rhs) {
      log_and_throw("Types on each side of assignment are incompatible:",
                    "type on LHS:",
                    *lhs_type->type,
                    ", type on RHS:",
                    *rhs_value->getType());
    }

    auto allocation = this->builder.CreateAlloca(lhs_type->type, /*ArraySize=*/nullptr, assgn.lhs);
    this->symbol_mapping.put(assgn.lhs, lhs_type->type, allocation);

    return this->builder.CreateStore(*converted_rhs, allocation);
  }

  BYTHON_VISITOR_IMPL(binary_operation, binop)
  {
    auto lhs_v = this->visit(*binop.lhs);

    if (m::matches(*binop.op, m::binary_operator {a::binop_tag::as})) {
      auto target = a::dyn_cast<ast::variable>(*binop.rhs);
      if (target == nullptr) {
        log_and_throw("`as` expression requires type identifier on RHS");
      }

      auto target_type = this->type_mapping.get(this->context, target->identifier);
      if (!target_type) {
        log_and_throw("Unknown type on RHS:", target->identifier);
      }

      auto conversion = codegen::convert(this->builder, lhs_v, target_type->type);
      if (!conversion) {
        log_and_throw("Unable to codegen conversion.",
                      "LHS:",
                      *lhs_v->getType(),
                      ", RHS:",
                      *target_type->type);
      }
      return *conversion;
    }

    else
    {
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
              log_and_throw("Unable to convert LHS type", *lhs_v->getType(), "to i1 (boolean)");
            }
            auto rhs_bool =
                codegen::convert(this->builder, rhs_v, llvm::Type::getInt1Ty(this->context));
            if (!rhs_bool) {
              log_and_throw("Unable to convert RHS type", *rhs_v->getType(), "to i1 (boolean)");
            }

            return this->builder.CreateLogicalAnd(*lhs_bool, *rhs_bool, "log.and");
          }
          case a::binop_tag::boolor: {
            auto lhs_bool =
                codegen::convert(this->builder, lhs_v, llvm::Type::getInt1Ty(this->context));
            if (!lhs_bool) {
              log_and_throw("Unable to convert LHS type", *lhs_v->getType(), "to i1 (boolean)");
            }
            auto rhs_bool =
                codegen::convert(this->builder, rhs_v, llvm::Type::getInt1Ty(this->context));
            if (!rhs_bool) {
              log_and_throw("Unable to convert RHS type", *rhs_v->getType(), "to i1 (boolean)");
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

      auto* op = ast::dyn_cast<comparison_operator>(*instance.ops[operand]);
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
    throw std::runtime_error {"Unknown AST Node: " + std::to_string(instance.tag().unwrap())};
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
};  // namespace bython

}  // namespace bython

namespace bython::codegen
{
auto compile(std::string_view name, std::unique_ptr<ast::node> ast, llvm::LLVMContext& context)
    -> std::unique_ptr<llvm::Module>
{
  auto module_ = std::make_unique<llvm::Module>(name, context);
  compile(std::move(ast), *module_);

  return module_;
}

auto compile(std::unique_ptr<ast::node> ast, llvm::Module& module_) -> void
{
  auto visitor = codegen_visitor {module_};
  visitor.visit(*ast);

  llvm::verifyModule(module_, &llvm::errs());
}
}  // namespace bython::codegen