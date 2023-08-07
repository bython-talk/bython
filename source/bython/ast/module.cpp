#include "module.hpp"

namespace bython::ast
{
mod::mod(statements body_)
    : body {std::move(body_)}
{
}

auto mod::tag() const -> ast::tag {
    return ast::tag{tag::mod};
}

}  // namespace bython::ast