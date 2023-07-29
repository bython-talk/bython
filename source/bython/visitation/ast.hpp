#pragma once

#include <cassert>

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

#define BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(DOWNCAST_TO, INST) \
  if (auto const* downcast = dyn_cast<DOWNCAST_TO>(INST)) { \
    BYTHON_DELEGATE(DOWNCAST_TO, *downcast) \
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

  virtual auto visit(expression const& inst) -> return_type final
  {
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(unary_operation, inst)
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(binary_operation, inst)

    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(comparison, inst)
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(variable, inst)

    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(call, inst)
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(integer, inst)

    assert(0);
  }

  // Statement classes
  BYTHON_MAKE_VISITOR_METHODS(type_definition, statement, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(assignment, statement, inst, return_type)

  virtual auto visit(statement const& inst) -> return_type final
  {
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(type_definition, inst)
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(assignment, inst)

    assert(0);
  }

  // Compound classes
  BYTHON_MAKE_VISITOR_METHODS(function_def, compound, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(for_, compound, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(while_, compound, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(conditional_branch, compound, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(unconditional_branch, compound, inst, return_type)

  virtual auto visit(compound const& inst) -> return_type final
  {
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(function_def, inst)
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(for_, inst)
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(while_, inst)
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(conditional_branch, inst)
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(unconditional_branch, inst)

    assert(0);
  }

  // Misc. TODO: make downcast methods
  BYTHON_MAKE_VISITOR_METHODS(unary_operator, node, inst, return_type)
  BYTHON_MAKE_VISITOR_METHODS(binary_operator, node, inst, return_type)

  // Fallthru via base classes
  BYTHON_VISITOR_DELEGATE(expression, node, inst, return_type)
  BYTHON_VISITOR_DELEGATE(statement, node, inst, return_type)
  BYTHON_VISITOR_DELEGATE(compound, node, inst, return_type)

  // "Biggest" fallthru
  virtual auto visit_node(node const& inst) -> RetTy = 0;
};

#undef BYTHON_DELEGATE
#undef BYTHON_VISITOR_DELEGATE
#undef BYTHON_VISITOR_DIRECT
#undef BYTHON_MAKE_VISITOR_METHODS
#undef BYTHON_VISITOR_DOWNCAST_AND_DISPATCH

#define BYTHON_VISITOR_IMPL(CLASS, INST, Body) \
  auto visit_##CLASS(CLASS const& INST)->return_type Body
}  // namespace bython::ast