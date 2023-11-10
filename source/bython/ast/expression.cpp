#include "expression.hpp"

#include <bython/ast/statement.hpp>

namespace bython::ast
{
unary_operation::unary_operation(unop_tag op_, std::unique_ptr<expression> rhs_)
    : op {std::make_unique<unary_operator>(op_)}
    , rhs {std::move(rhs_)}
{
}

auto unary_operation::tag() const -> ast::tag
{
  return ast::tag {tag::unary_operation};
}

binary_operation::binary_operation(std::unique_ptr<expression> lhs_,
                                   binop_tag binop_,
                                   std::unique_ptr<expression> rhs_)
    : lhs {std::move(lhs_)}
    , rhs {std::move(rhs_)}
    , op {std::make_unique<binary_operator>(binop_)}
{
}

auto binary_operation::tag() const -> ast::tag
{
  return ast::tag {tag::binary_operation};
}

comparison::comparison(ast::expressions operands_, std::vector<comparison_operator_tag> ops_)
    : operands {std::move(operands_)}
    , ops {}
{
  for (auto&& op : ops_) {
    this->add_operator(op);
  }
}

auto comparison::tag() const -> ast::tag
{
  return ast::tag {tag::comparison};
}

auto comparison::add_operator(bython::ast::comparison_operator_tag op) -> void
{
  this->ops.emplace_back(std::make_unique<comparison_operator>(op));
}

auto comparison::add_operand(std::unique_ptr<expression> expr) -> void
{
  this->operands.emplace_back(std::move(expr));
}

argument_list::argument_list(ast::expressions arguments_)
    : arguments {std::move(arguments_)}
{
}

auto argument_list::tag() const -> ast::tag {
  return ast::tag{tag::argument_list};
}

call::call(std::string callee_, argument_list arguments_)
    : callee {std::move(callee_)}
    , arguments {std::move(arguments_)}
{
}

auto call::tag() const -> ast::tag
{
  return ast::tag {tag::call};
}

variable::variable(std::string identifier_)
    : identifier {std::move(identifier_)}
{
}

auto variable::tag() const -> ast::tag
{
  return ast::tag {tag::variable};
}

integer::integer(int64_t value_)
    : value {value_}
{
}

auto integer::tag() const -> ast::tag
{
  return ast::tag {tag::integer};
}

}  // namespace bython::ast