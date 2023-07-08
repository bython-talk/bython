#pragma once

#include <vector>

#include "bases.hpp"
#include "statement.hpp"

namespace bython::ast
{
struct compound : statement
{
  explicit compound(statements body_)
      : body {std::move(body_)}
  {
  }

  statements body;
};

struct for_ : compound
{
};

struct while_ : compound
{
};

struct if_ : compound
{
};

struct parameter
{
  std::string name;
};

using parameters = std::vector<parameter>;

struct function_def : compound
{
  function_def(std::string name_,
               std::vector<parameter> parameters_,
               statements body_)
      : compound(std::move(body_))
      , name(std::move(name_))
      , parameters(std::move(parameters_))
  {
  }

  std::string name;
  std::vector<parameter> parameters;
};

};  // namespace bython::ast
