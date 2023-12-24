#include "expression.hpp"

#include <bython/ast/statement.hpp>

namespace bython::ast
{
unary_operation::unary_operation(unop_tag op_, std::unique_ptr<expression> rhs_)
    : op {op_}
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
    , op {binop_}
    , rhs {std::move(rhs_)}
{
}

auto binary_operation::tag() const -> ast::tag
{
  return ast::tag {tag::binary_operation};
}

comparison::comparison(std::unique_ptr<expression> lhs_,
                       ast::comparison_operator_tag comp_op,
                       std::unique_ptr<expression> rhs_)
    : lhs {std::move(lhs_)}
    , op {std::move(comp_op)}
    , rhs {std::move(rhs_)}
{
}

auto comparison::tag() const -> ast::tag
{
  return ast::tag {tag::comparison};
}

argument_list::argument_list(ast::expressions arguments_)
    : arguments {std::move(arguments_)}
{
}

auto argument_list::tag() const -> ast::tag
{
  return ast::tag {tag::argument_list};
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

signed_integer::signed_integer(int64_t value_)
    : value {value_}
{
}

auto signed_integer::tag() const -> ast::tag
{
  return ast::tag {tag::signed_integer};
}

unsigned_integer::unsigned_integer(uint64_t value_)
    : value {value_}
{
}

auto unsigned_integer::tag() const -> ast::tag
{
  return ast::tag {tag::unsigned_integer};
}

}  // namespace bython::ast