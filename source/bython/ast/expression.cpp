#include "expression.hpp"

#include <bython/visitation.hpp>

namespace bython::ast
{
unary_operation::unary_operation(unary_operator op_,
                                 std::unique_ptr<expression> rhs_)
    : op {op_}
    , rhs {std::move(rhs_)}
{
}

auto unary_operation::visit(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

binary_operation::binary_operation(std::unique_ptr<expression> lhs_,
                                   binary_operator binop_,
                                   std::unique_ptr<expression> rhs_)
    : op(binop_)
    , lhs {std::move(lhs_)}
    , rhs {std::move(rhs_)}
{
}

auto binary_operation::visit(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

comparison::comparison(ast::expressions operands_,
                       std::vector<comparison_operator> ops_)
    : operands {std::move(operands_)}
    , ops {std::move(ops_)}
{
}

auto comparison::visit(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

call::call(std::string callee_,
           std::vector<std::unique_ptr<expression>> arguments_)
    : callee {std::move(callee_)}
    , arguments {std::move(arguments_)}
{
}

auto call::visit(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

variable::variable(std::string identifier_)
    : identifier {std::move(identifier_)}
{
}

auto variable::visit(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

integer::integer(int64_t value_)
    : value {value_}
{
}

auto integer::visit(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

}  // namespace bython::ast