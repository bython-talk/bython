#include <algorithm>

#include "special.hpp"

#include "bases.hpp"

namespace bython::matching
{
auto do_not_care::matches(const ast::node& /*ast*/) const -> bool
{
  return true;
}

auto one_of::matches(const ast::node& ast) const -> bool
{
  return std::any_of(this->matchers.begin(),
                     this->matchers.end(),
                     [&](auto const& match) { return matching::matches(ast, *match); });
}

auto operator|(std::unique_ptr<matcher> lhs, std::unique_ptr<matcher> rhs)
    -> std::unique_ptr<matcher>
{
  auto* lhs_one_of_inst = dynamic_cast<one_of*>(&*lhs);
  auto* rhs_one_of_inst = dynamic_cast<one_of*>(&*rhs);

  if (lhs_one_of_inst != nullptr && rhs_one_of_inst != nullptr) {
    lhs_one_of_inst->matchers.insert(lhs_one_of_inst->matchers.end(),
                                     std::make_move_iterator(rhs_one_of_inst->matchers.begin()),
                                     std::make_move_iterator(rhs_one_of_inst->matchers.end()));
    return std::make_unique<one_of>(std::move(*lhs_one_of_inst));
  }

  if (lhs_one_of_inst != nullptr) {
    lhs_one_of_inst->matchers.emplace_back(std::move(rhs));
    return std::make_unique<one_of>(std::move(*lhs_one_of_inst));
  }

  if (rhs_one_of_inst != nullptr) {
    rhs_one_of_inst->matchers.emplace_back(std::move(lhs));
    return std::make_unique<one_of>(std::move(*rhs_one_of_inst));
  }

  auto matchers = std::vector<std::unique_ptr<matcher>> {};
  matchers.emplace_back(std::move(lhs));
  matchers.emplace_back(std::move(rhs));

  return std::make_unique<one_of>(std::move(matchers));
}

}  // namespace bython::matching