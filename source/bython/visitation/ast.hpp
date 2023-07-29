#pragma once

namespace bython::ast
{

// Based on https://llvm.org/doxygen/InstVisitor_8h_source.html
#define BYTHON_DELEGATE(DELEGATE_TO, INST) \
  { \
    auto downcast_visitor = static_cast<SubClass*>(this); \
    return downcast_visitor->visit_##DELEGATE_TO(INST); \
  }

#define BYTHON_VISITOR_DELEGATE(CLASS, DELEGATE_TO, INST, RET) \
  auto visit_##CLASS(struct CLASS const& INST)->RET \
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
  BYTHON_MAKE_VISITOR_METHODS(integer, expression, inst, return_type)

  // Statement classes
  BYTHON_MAKE_VISITOR_METHODS(type_definition, statement, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(assignment, statement, inst, return_type)

  // Compound classes
  BYTHON_MAKE_VISITOR_METHODS(function_def, compound, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(for_, compound, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(while_, compound, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(conditional_branch, compound, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(unconditional_branch, compound, inst, return_type)

  // Misc.
  BYTHON_MAKE_VISITOR_METHODS(unary_operator, node, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(binary_operator, node, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(mod, node, inst, return_type)

  // Fallthru via base classes
  BYTHON_MAKE_VISITOR_METHODS(expression, node, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(statement, node, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(compound, node, inst, return_type)

  // "Biggest" fallthru
  virtual auto visit_node(node const& inst) -> RetTy = 0;
};

#undef BYTHON_DELEGATE
#undef BYTHON_VISITOR_DELEGATE
#undef BYTHON_VISITOR_DIRECT
#undef BYTHON_MAKE_VISITOR_METHODS

#define BYTHON_VISITOR_IMPL(CLASS, INST, Body) \
  auto visit_##CLASS(CLASS const& INST)->return_type Body
}  // namespace bython::ast