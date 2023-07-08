#pragma once

#include <memory>
#include <string>
#include <vector>

#include "bases.hpp"

namespace bython::ast
{
struct expression : node
{
};

struct binary_operation : expression
{
  enum class binary_operator
  {
    plus,
    minus,
    multiply,
    divide,
  } binop;

  binary_operation(binary_operator binop_,
                   std::unique_ptr<expression> lhs_,
                   std::unique_ptr<expression> rhs_)
      : expression {}
      , binop(binop_)
      , lhs {std::move(lhs_)}
      , rhs {std::move(rhs_)}
  {
  }

  std::unique_ptr<expression> lhs, rhs;
};

struct variable : expression
{
  explicit variable(std::string identifier_)
      : identifier {std::move(identifier_)}
  {
  }

  std::string identifier;
};

struct call : expression
{
  call(std::string callee_, std::vector<std::unique_ptr<expression>> arguments_)
      : callee {std::move(callee_)}
      , arguments {std::move(arguments_)}
  {
  }

  std::string callee;
  std::vector<std::unique_ptr<expression>> arguments;
};

}  // namespace bython::ast
