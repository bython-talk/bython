#include "compound.hpp"

#include <bython/visitation.hpp>

namespace bython::ast
{
compound::compound(statements body_)
    : body {std::move(body_)}
{
}

auto for_::accept(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

auto while_::accept(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
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

auto conditional_branch::accept(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

unconditional_branch::unconditional_branch(statements body_)
    : compound {std::move(body_)}
{
}

auto unconditional_branch::accept(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

auto function_def::accept(visitation::visitor& visitor) const -> void
{
  visitor.visit(*this);
}

}  // namespace bython::ast