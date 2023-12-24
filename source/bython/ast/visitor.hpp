#pragma once

#include <stdexcept>

#include <bython/ast/tags.hpp>

namespace bython::ast
{

// Based on https://llvm.org/doxygen/InstVisitor_8h_source.html
#define BYTHON_DELEGATE(DELEGATE_TO, INST) \
  { \
    auto downcast_visitor = static_cast<SubClass*>(this); \
    return downcast_visitor->visit_##DELEGATE_TO(INST); \
  }

#define BYTHON_VISITOR_DELEGATE(CLASS, DELEGATE_TO, INST, RET) \
  virtual auto visit_##CLASS(struct CLASS const& INST)->RET \
  { \
    BYTHON_DELEGATE(DELEGATE_TO, INST); \
  }

#define BYTHON_VISITOR_DIRECT(CLASS, INST, RET) \
  virtual auto visit(CLASS const& INST)->RET final \
  { \
    BYTHON_DELEGATE(CLASS, INST); \
  }

#define BYTHON_MAKE_VISITOR_METHODS(CLASS, DELEGATE_TO, INST, RET) \
  BYTHON_VISITOR_DIRECT(CLASS, INST, RET) \
  BYTHON_VISITOR_DELEGATE(CLASS, DELEGATE_TO, INST, RET)

#define BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(DOWNCAST_TO, BASE, INST) \
  case tag::DOWNCAST_TO: { \
    auto* downcast = dyn_cast<DOWNCAST_TO>(INST); \
    BYTHON_DELEGATE(DOWNCAST_TO, *downcast) \
  }

template<typename SubClass, typename RetTy = void>
struct visitor
{
  using return_type = RetTy;

  virtual ~visitor() = default;

  // Expression classes
  BYTHON_MAKE_VISITOR_METHODS(unary_operation, expression, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(binary_operation, expression, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(comparison, expression, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(variable, expression, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(call, expression, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(signed_integer, expression, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(unsigned_integer, expression, inst, return_type)

  BYTHON_VISITOR_DELEGATE(expression, node, inst, return_type)
  virtual auto visit(expression const& inst) -> return_type final
  {
    switch (auto t = inst.tag(); tag::expression {t.unwrap()}) {
      default:
        throw std::logic_error("Unrecognised expression tag");
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(unary_operation, expression, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(binary_operation, expression, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(comparison, expression, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(variable, expression, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(call, expression, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(signed_integer, expression, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(unsigned_integer, expression, inst)
    }
  }

  // Statement classes
  BYTHON_MAKE_VISITOR_METHODS(type_definition, statement, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(let_assignment, statement, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(expression_statement, statement, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(for_, statement, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(while_, statement, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(conditional_branch, statement, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(unconditional_branch, statement, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(function_def, statement, inst, return_type)

  BYTHON_MAKE_VISITOR_METHODS(return_, statement, inst, return_type)

  BYTHON_VISITOR_DELEGATE(statement, node, inst, return_type)
  virtual auto visit(statement const& inst) -> return_type final
  {
    switch (auto t = inst.tag(); tag::statement {t.unwrap()}) {
      default:
        throw std::logic_error("Unrecognised statement tag");
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(type_definition, statement, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(let_assignment, statement, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(expression_statement, statement, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(for_, statement, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(while_, statement, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(conditional_branch, statement, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(unconditional_branch, statement, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(function_def, statement, inst)
        BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(return_, statement, inst)
    }
  }

  BYTHON_MAKE_VISITOR_METHODS(mod, node, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(unary_operator, node, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(binary_operator, node, inst, return_type)

  // Fallthru to bottom
  virtual auto visit(node const& inst) -> return_type final
  {
    if (auto t = inst.tag(); t.is_expression()) {
      auto* e = ast::dyn_cast<expression>(inst);
      return this->visit(*e);
    } else if (t.is_statement()) {
      auto* s = ast::dyn_cast<statement>(inst);
      return this->visit(*s);
    } else {
      // Manually handle final entries
      switch (tag::misc {t.unwrap()}) {
        default: {
          throw std::logic_error("Unrecognised misc tag");
        }
        case tag::unary_operator: {
          auto* m = ast::dyn_cast<unary_operator>(inst);
          return this->visit(*m);
        }
        case tag::binary_operator: {
          auto* m = ast::dyn_cast<binary_operator>(inst);
          return this->visit(*m);
        }
        case tag::comparison_operator: {
          auto* m = ast::dyn_cast<comparison_operator>(inst);
          return this->visit(*m);
        }

        case tag::mod: {
          auto* m = ast::dyn_cast<mod>(inst);
          return this->visit(*m);
        }
      }
    }
  }
  virtual auto visit_node(node const& inst) -> RetTy = 0;
};

#undef BYTHON_DELEGATE
#undef BYTHON_VISITOR_DELEGATE
#undef BYTHON_VISITOR_DIRECT
#undef BYTHON_MAKE_VISITOR_METHODS
#undef BYTHON_VISITOR_DOWNCAST_AND_DISPATCH

#define BYTHON_VISITOR_IMPL(CLASS, INST) auto visit_##CLASS(CLASS const& INST)->return_type final
}  // namespace bython::ast