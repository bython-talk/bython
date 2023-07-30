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
  virtual auto visit_##CLASS(struct CLASS const& INST)->RET \
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

/*
 * NOTE: Technically correct, but CLion tooling is not able
 * NOTE: to make use of the declared method for autogenerated overriding
 * NOTE: So we disable it for now
 */
#if 0
#  define BYTHON_MAKE_VISITOR_METHODS(CLASS, DELEGATE_TO, INST, RET) \
    BYTHON_VISITOR_DIRECT(CLASS, INST, RET) \
    BYTHON_VISITOR_DELEGATE(CLASS, DELEGATE_TO, INST, RET)
#endif

template<typename SubClass, typename RetTy = void>
struct visitor
{
  using return_type = RetTy;

  virtual ~visitor() = default;

  // Expression classes
  BYTHON_VISITOR_DIRECT(unary_operation, inst, return_type)
  BYTHON_VISITOR_DELEGATE(unary_operation, expression, inst, return_type)

  BYTHON_VISITOR_DIRECT(binary_operation, inst, return_type)
  BYTHON_VISITOR_DELEGATE(binary_operation, expression, inst, return_type)

  BYTHON_VISITOR_DIRECT(comparison, inst, return_type)
  BYTHON_VISITOR_DELEGATE(comparison, expression, inst, return_type)

  BYTHON_VISITOR_DIRECT(variable, inst, return_type)
  BYTHON_VISITOR_DELEGATE(variable, expression, inst, return_type)

  BYTHON_VISITOR_DIRECT(call, inst, return_type)
  BYTHON_VISITOR_DELEGATE(call, expression, inst, return_type)

  BYTHON_VISITOR_DIRECT(integer, inst, return_type)
  BYTHON_VISITOR_DELEGATE(integer, expression, inst, return_type)

  BYTHON_VISITOR_DELEGATE(expression, node, inst, return_type)
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
  BYTHON_VISITOR_DIRECT(type_definition, inst, return_type)
  BYTHON_VISITOR_DELEGATE(type_definition, statement, inst, return_type)

  BYTHON_VISITOR_DIRECT(assignment, inst, return_type)
  BYTHON_VISITOR_DELEGATE(assignment, statement, inst, return_type)

  BYTHON_VISITOR_DELEGATE(statement, node, inst, return_type)
  virtual auto visit(statement const& inst) -> return_type final
  {
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(type_definition, inst)
    BYTHON_VISITOR_DOWNCAST_AND_DISPATCH(assignment, inst)

    assert(0);
  }

  // Compound classes
  BYTHON_VISITOR_DIRECT(function_def, inst, return_type)
  BYTHON_VISITOR_DELEGATE(function_def, compound, inst, return_type)

  BYTHON_VISITOR_DIRECT(for_, inst, return_type)
  BYTHON_VISITOR_DELEGATE(for_, compound, inst, return_type)

  BYTHON_VISITOR_DIRECT(while_, inst, return_type)
  BYTHON_VISITOR_DELEGATE(while_, compound, inst, return_type)

  BYTHON_VISITOR_DIRECT(conditional_branch, inst, return_type)
  BYTHON_VISITOR_DELEGATE(conditional_branch, compound, inst, return_type)

  BYTHON_VISITOR_DIRECT(unconditional_branch, inst, return_type)
  BYTHON_VISITOR_DELEGATE(unconditional_branch, compound, inst, return_type)

  BYTHON_VISITOR_DELEGATE(compound, node, inst, return_type)
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
  BYTHON_VISITOR_DIRECT(unary_operator, inst, return_type)
  BYTHON_VISITOR_DELEGATE(unary_operator, node, inst, return_type)

  BYTHON_VISITOR_DIRECT(binary_operator, inst, return_type)
  BYTHON_VISITOR_DELEGATE(binary_operator, node, inst, return_type)

  // Fallthru to bottom
  virtual auto visit_node(node const& inst) -> RetTy = 0;
};

#undef BYTHON_DELEGATE
#undef BYTHON_VISITOR_DELEGATE
#undef BYTHON_VISITOR_DIRECT
#undef BYTHON_MAKE_VISITOR_METHODS
#undef BYTHON_VISITOR_DOWNCAST_AND_DISPATCH

#define BYTHON_VISITOR_IMPL(CLASS, INST) \
  auto visit_##CLASS(CLASS const& INST)->return_type final
}  // namespace bython::ast