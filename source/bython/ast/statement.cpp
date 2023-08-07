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

}  // namespace bython::ast