#pragma once

#include <memory>
#include <vector>

#include "bases.hpp"
#include "special.hpp"

namespace bython::matching
{
struct do_not_care : matcher
{
  auto matches(const ast::node& ast) const -> bool override;
};

struct one_of : matcher
{
  explicit one_of(std::vector<std::unique_ptr<matcher>> matchers_)
      : matchers {std::move(matchers_)}
  {
  }

  template<typename InputIterator>
  one_of(InputIterator from, InputIterator to)
      : matchers {std::make_move_iterator(from), std::make_move_iterator(to)}
  {
  }

  auto matches(const ast::node& ast) const -> bool override;

  std::vector<std::unique_ptr<matcher>> matchers;
};

auto operator|(std::unique_ptr<matcher> lhs, std::unique_ptr<matcher> rhs)
    -> std::unique_ptr<matcher>;

}  // namespace bython::matching