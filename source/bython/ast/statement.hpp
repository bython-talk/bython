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

struct type_definition : statement
{
  type_definition(std::string identifier_, type_definition_stmts body_)
      : identifier {std::move(identifier_)}
      , body {std::move(body_)}
  {
  }

  std::string identifier;
  type_definition_stmts body;
};

struct assignment : statement
{
  assignment(std::string lhs_, std::unique_ptr<expression> rhs_)
      : lhs {std::move(lhs_)}
      , rhs {std::move(rhs_)}
  {
  }

  std::string lhs;
  std::unique_ptr<expression> rhs;
};

}  // namespace bython::ast