#include <bython/ast.hpp>
#include <bython/visitation/ast.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace bython::ast;

TEST_CASE("Unary Operation", "[Visitation]")
{
  struct unary_operation_visitor final : visitor<unary_operation_visitor, bool>
  {
    BYTHON_VISITOR_IMPL(unary_operation, /*_*/)
    {
      return true;
    }
    BYTHON_VISITOR_IMPL(node, /*_*/)
    {
      return false;
    }
  };

  auto node = unary_operation {unop_tag::plus, std::make_unique<variable>("v")};
  REQUIRE(unary_operation_visitor {}.visit(node));
}

TEST_CASE("Binary Operation", "[Visitation]")
{
  struct binary_operation_visitor final
      : visitor<binary_operation_visitor, bool>
  {
    BYTHON_VISITOR_IMPL(binary_operation, /*_*/)
    {
      return true;
    }
    BYTHON_VISITOR_IMPL(node, /*_*/)
    {
      return false;
    }
  };

  auto node = binary_operation {std::make_unique<variable>("lhs"),
                                binop_tag::plus,
                                std::make_unique<variable>("rhs")};
  REQUIRE(binary_operation_visitor {}.visit(node));
}
