#include "visitor.hpp"

namespace bython::visitation
{

auto visitor::visit(const ast::mod&) -> visitor::traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::variable const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::integer const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::comparison const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::comparison_operator const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::binary_operation const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::binary_operator const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::unary_operation const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::unary_operator const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::call const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::if_expression const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::assignment const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::type_definition const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::for_ const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::while_ const&) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::function_def const&) -> traversal
{
  return traversal::CONTINUE;
}

}  // namespace bython::visitation