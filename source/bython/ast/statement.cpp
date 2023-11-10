#include "statement.hpp"

namespace bython::ast
{
assignment::assignment(std::string lhs_, std::string hint_, std::unique_ptr<expression> rhs_)
    : lhs {std::move(lhs_)}
    , hint {std::move(hint_)}
    , rhs {std::move(rhs_)}
{
}

auto assignment::tag() const -> ast::tag
{
  return ast::tag {tag::assignment};
}

type_definition::type_definition(std::string identifier_, bython::ast::type_definition_stmts body_)
    : identifier {std::move(identifier_)}
    , body {std::move(body_)}
{
}

auto type_definition::tag() const -> ast::tag
{
  return ast::tag {tag::type_definition};
}

expression_statement::expression_statement(std::unique_ptr<expression> discarded_)
    : discarded {std::move(discarded_)}
{
}

auto expression_statement::tag() const -> ast::tag
{
  return ast::tag {tag::expression_statement};
}

for_::for_(statements body_)
    : body {std::move(body_)}
{
}

auto for_::tag() const -> ast::tag
{
  return ast::tag {tag::for_};
}

while_::while_(statements body_)
    : body {std::move(body_)}
{
}

auto while_::tag() const -> ast::tag
{
  return ast::tag {tag::while_};
}

conditional_branch::conditional_branch(std::unique_ptr<expression> condition_, statements body_)
    : condition {std::move(condition_)}
    , orelse {nullptr}
    , body {std::move(body_)}
{
}

conditional_branch::conditional_branch(std::unique_ptr<expression> condition_,
                                       statements body_,
                                       std::unique_ptr<statement> orelse_)
    : condition {std::move(condition_)}
    , orelse {std::move(orelse_)}
    , body {std::move(body_)}
{
}

auto conditional_branch::tag() const -> ast::tag
{
  return ast::tag {tag::conditional_branch};
}

unconditional_branch::unconditional_branch(statements body_)
    : body {std::move(body_)}
{
}

auto unconditional_branch::tag() const -> ast::tag
{
  return ast::tag {tag::unconditional_branch};
}

auto parameter::tag() const -> ast::tag {
  return ast::tag{tag::parameter};
}

auto parameter_list::tag() const -> ast::tag {
  return ast::tag{tag::parameter_list};
}

function_def::function_def(std::string name_, parameter_list parameters_, statements body_)
    : name(std::move(name_))
    , parameters(std::move(parameters_))
    , body(std::move(body_))
{
}

auto function_def::tag() const -> ast::tag
{
  return ast::tag {tag::function_def};
}

return_::return_(std::unique_ptr<expression> expr_)  : expr{std::move(expr_)} {}

auto return_::tag() const -> ast::tag {
  return ast::tag{tag::return_};
}

}  // namespace bython::ast