#include "expression.hpp"

#include <bython/ast/statement.hpp>

namespace bython::ast
{
unary_operation::unary_operation(unop_tag op_, std::unique_ptr<expression> rhs_)
    : op {std::make_unique<unary_operator>(op_)}
    , rhs {std::move(rhs_)}
{
}

binary_operation::binary_operation(std::unique_ptr<expression> lhs_,
                                   binop_tag binop_,
                                   std::unique_ptr<expression> rhs_)
    : lhs {std::move(lhs_)}
    , rhs {std::move(rhs_)}
    , op {std::make_unique<binary_operator>(binop_)}
{
}

comparison::comparison(ast::expressions operands_,
                       std::vector<comparison_operator_tag> ops_)
    : operands {std::move(operands_)}
    , ops {}
{
  for (auto&& op : ops_) {
    this->add_operator(op);
  }
}

auto comparison::add_operator(bython::ast::comparison_operator_tag op) -> void
{
  this->ops.emplace_back(std::make_unique<comparison_operator>(op));
}

auto comparison::add_operand(std::unique_ptr<expression> expr) -> void
{
  this->operands.emplace_back(std::move(expr));
}

call::call(std::string callee_,
           std::vector<std::unique_ptr<expression>> arguments_)
    : callee {std::move(callee_)}
    , arguments {std::move(arguments_)}
{
}

variable::variable(std::string identifier_)
    : identifier {std::move(identifier_)}
{
}

integer::integer(int64_t value_)
    : value {value_}
{
}

}  // namespace bython::ast