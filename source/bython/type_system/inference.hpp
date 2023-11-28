#pragma once

#include <map>
#include <optional>

#include <boost/uuid/uuid.hpp>

#include "builtin.hpp"
#include "bython/ast/expression.hpp"
#include "environment.hpp"

namespace bython::type_system
{
auto try_infer_impl(ast::expression const& expr, type_system::environment const& environment)
    -> std::optional<type_system::type*>;
}