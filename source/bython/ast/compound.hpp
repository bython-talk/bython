#pragma once

#include <vector>

#include "bases.hpp"
#include "statement.hpp"

namespace bython::ast
{
struct compound : statement
{
  explicit compound(statements body_);

  statements body;
};

struct for_ final : compound
{
  /*auto transform(visitation::transformer& transformer) const
      -> std::unique_ptr<node> override;*/
};

struct while_ final : compound
{
};

struct conditional_branch final : compound
{
  conditional_branch(std::unique_ptr<expression> condition_, statements body_);
  conditional_branch(std::unique_ptr<expression> condition_,
                     statements body_,
                     std::unique_ptr<compound> orelse_);

  std::unique_ptr<node> condition;
  std::unique_ptr<node> orelse;
};

struct unconditional_branch final : compound
{
  explicit unconditional_branch(statements body_);
};

struct parameter
{
  std::string name;
};

using parameters = std::vector<parameter>;

struct function_def final : compound
{
  function_def(std::string name_, std::vector<parameter> parameters_, statements body_)
      : compound(std::move(body_))
      , name(std::move(name_))
      , parameters(std::move(parameters_))
  {
  }

  std::string name;
  std::vector<parameter> parameters;
};

}  // namespace bython::ast
