#include "compound.hpp"

namespace bython::ast
{
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

function_def::function_def(std::string name_, std::vector<parameter> parameters_, statements body_)
    : name(std::move(name_))
    , parameters(std::move(parameters_))
    , body(std::move(body_))
{
}

auto function_def::tag() const -> ast::tag
{
  return ast::tag {tag::function_def};
}

}  // namespace bython::ast