#include "statement.hpp"

#include <bython/visitation.hpp>

namespace bython::ast
{
assignment::assignment(std::string lhs_, std::unique_ptr<expression> rhs_)
    : lhs {std::move(lhs_)}
    , rhs {std::move(rhs_)}
{
}

auto assignment::accept(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

type_definition::type_definition(std::string identifier_,
                                 bython::ast::type_definition_stmts body_)
    : identifier {std::move(identifier_)}
    , body {std::move(body_)}
{
}

auto type_definition::accept(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

}  // namespace bython::ast