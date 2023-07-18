#pragma once

#include <bython/ast.hpp>

namespace bython::visitation
{
struct visitor
{
  enum class traversal
  {
    CONTINUE,
    TERMINATE,
  };

  virtual ~visitor() = default;

  // module
  virtual auto visit(ast::mod const& module) -> traversal;

  // expression
  virtual auto visit(ast::variable const& variable) -> traversal;

  virtual auto visit(ast::integer const& integer) -> traversal;

  virtual auto visit(ast::comparison_operator const& comparison_operator)
      -> traversal;

  virtual auto visit(ast::comparison const& comparison) -> traversal;

  virtual auto visit(ast::binary_operator const& binary_operator) -> traversal;

  virtual auto visit(ast::binary_operation const& binop) -> traversal;

  virtual auto visit(ast::unary_operator const& unary_operator) -> traversal;

  virtual auto visit(ast::unary_operation const& unop) -> traversal;

  virtual auto visit(ast::call const& call) -> traversal;

  // statement
  virtual auto visit(ast::assignment const& assignment) -> traversal;

  virtual auto visit(ast::type_definition const& type_definition) -> traversal;

  // branching
  virtual auto visit(ast::if_expression const& if_expression) -> traversal;
  virtual auto visit(ast::else_expression const& else_expression) -> traversal;
  virtual auto visit(ast::branching_body const& branching_body) -> traversal;

  // compound statement
  virtual auto visit(ast::for_ const& for_) -> traversal;

  virtual auto visit(ast::while_ const& while_) -> traversal;

  virtual auto visit(ast::function_def const& function) -> traversal;
};
}  // namespace bython::visitation