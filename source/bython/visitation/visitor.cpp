#include "visitor.hpp"

namespace bython::visitation
{

auto visitor::visit(const ast::mod&) -> visitor::traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::variable const& variable) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::integer const& integer) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::comparison const& comparison) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::binary_operation const& binary_op) -> traversal
{
  return traversal::CONTINUE;
}
auto visitor::visit(ast::unary_operation const& unary_operation) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::call const& call) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::assignment const& assignment) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::type_definition const& type_definition) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::for_ const& for_) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::while_ const& while_) -> traversal
{
  return traversal::CONTINUE;
}

auto visitor::visit(ast::function_def const& function) -> traversal
{
  return traversal::CONTINUE;
}

}  // namespace bython::visitation