#include "statement.hpp"

namespace bython::ast
{
assignment::assignment(std::string lhs_, std::string hint_, std::unique_ptr<expression> rhs_)
    : lhs {std::move(lhs_)}
    , hint {std::move(hint_)}
    , rhs {std::move(rhs_)}
{
}

type_definition::type_definition(std::string identifier_, bython::ast::type_definition_stmts body_)
    : identifier {std::move(identifier_)}
    , body {std::move(body_)}
{
}

expression_statement::expression_statement(std::unique_ptr<expression> discarded_)
    : discarded {std::move(discarded_)}
{
}

}  // namespace bython::ast