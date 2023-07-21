#include "compound.hpp"

#include <bython/visitation.hpp>

namespace bython::ast
{
compound::compound(statements body_)
    : body {std::move(body_)}
{
}

conditional_branch::conditional_branch(std::unique_ptr<expression> condition_,
                                       statements body_)
    : compound {std::move(body_)}
    , condition {std::move(condition_)}
    , orelse {nullptr}
{
}

conditional_branch::conditional_branch(std::unique_ptr<expression> condition_,
                                       statements body_,
                                       std::unique_ptr<compound> orelse_)
    : compound {std::move(body_)}
    , condition {std::move(condition_)}
    , orelse {std::move(orelse_)}
{
}

unconditional_branch::unconditional_branch(statements body_)
    : compound {std::move(body_)}
{
}

}  // namespace bython::ast