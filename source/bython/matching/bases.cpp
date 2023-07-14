#include "bases.hpp"

#include <bython/visitation/visitor.hpp>

namespace bython
{

struct matching_visitor final : visitation::visitor
{
  explicit matching_visitor(matching::matcher const& matcher_)
      : matcher {matcher_}
  {
  }

  auto visit(const ast::variable& variable) -> traversal override
  {
    return visitor::visit(variable);
  }

  auto visit(const ast::integer& integer) -> traversal override
  {
    return visitor::visit(integer);
  }

  auto visit(const ast::comparison& binop) -> traversal override
  {
    return visitor::visit(binop);
  }

  auto visit(const ast::binary_operation& binop) -> traversal override
  {
    return visitor::visit(binop);
  }

  auto visit(const ast::unary_operation& unop) -> traversal override
  {
    return visitor::visit(unop);
  }

  auto visit(const ast::call& call) -> traversal override
  {
    return visitor::visit(call);
  }

  auto visit(const ast::assignment& assignment) -> traversal override
  {
    return visitor::visit(assignment);
  }

  auto visit(const ast::type_definition& type_definition) -> traversal override
  {
    return visitor::visit(type_definition);
  }

  auto visit(const ast::for_& for_) -> traversal override
  {
    return visitor::visit(for_);
  }

  auto visit(const ast::while_& while_) -> traversal override
  {
    return visitor::visit(while_);
  }

  auto visit(const ast::function_def& function) -> traversal override
  {
    return visitor::visit(function);
  }

  matching::matcher const& matcher;
  bool matches = true;
};

}  // namespace bython

namespace bython::matching
{

auto matches(std::unique_ptr<ast::node> const& ast,
             std::unique_ptr<matcher> const& matcher) -> bool
{
  return matches(*ast, *matcher);
}

auto matches(ast::node const& ast, matcher const& matcher) -> bool
{
    return matcher.matches(ast);
}
}  // namespace bython::matching