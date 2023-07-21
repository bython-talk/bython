#pragma once

#include <memory>
#include <string>
#include <vector>

#include "bases.hpp"
#include "expression.hpp"

namespace bython::ast
{
struct statement : node
{
};

using statements = std::vector<std::unique_ptr<statement>>;

struct type_definition_stmt
{
  std::string identifier;
};
using type_definition_stmts = std::vector<type_definition_stmt>;

struct type_definition final : statement
{
  type_definition(std::string identifier_, type_definition_stmts body_);

  std::string identifier;
  type_definition_stmts body;
};

struct assignment final : statement
{
  assignment(std::string lhs_, std::unique_ptr<expression> rhs_);

  std::string lhs;
  std::unique_ptr<expression> rhs;
};

}  // namespace bython::ast