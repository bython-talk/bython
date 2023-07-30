#include "module.hpp"

namespace bython::ast
{
mod::mod(statements body_)
    : body {std::move(body_)}
{
}

}  // namespace bython::ast