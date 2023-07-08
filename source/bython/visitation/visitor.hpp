#pragma once

#include <bython/ast/expression.hpp>

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

  virtual auto visit(ast::variable const& variable) -> traversal
  {
    return traversal::CONTINUE;
  }

  virtual auto visit(ast::binary_operation const& binop) -> traversal
  {
    return traversal::CONTINUE;
  }
};
};  // namespace bython::visitation