#include "statement.hpp"

namespace bython::ast
{
let_assignment::let_assignment(std::string lhs_, std::string hint_, std::unique_ptr<expression> rhs_)
    : lhs {std::move(lhs_)}
    , hint {std::move(hint_)}
    , rhs {std::move(rhs_)}
{
}

auto let_assignment::tag() const -> ast::tag
{
  return ast::tag {tag::let_assignment};
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

parameter::parameter(std::string name_, std::string hint_)
    : name {std::move(name_)}
    , hint {std::move(hint_)}
{
}

auto parameter::tag() const -> ast::tag
{
  return ast::tag {tag::parameter};
}

parameter_list::parameter_list(std::vector<parameter> parameters_)
    : parameters {std::move(parameters_)}
{
}

auto parameter_list::tag() const -> ast::tag
{
  return ast::tag {tag::parameter_list};
}

signature::signature(std::string name_,
                     parameter_list parameters_,
                     std::optional<std::string> rettype_)
    : name {std::move(name_)}
    , parameters {std::move(parameters_)}
    , rettype {std::move(rettype_)}
{
}

function_def::function_def(signature sig, statements body_)
    : sig(std::move(sig))
    , body(std::move(body_))
{
}

auto function_def::tag() const -> ast::tag
{
  return ast::tag {tag::function_def};
}

return_::return_(std::unique_ptr<expression> expr_)
    : expr {std::move(expr_)}
{
}

auto return_::tag() const -> ast::tag
{
  return ast::tag {tag::return_};
}

}  // namespace bython::ast