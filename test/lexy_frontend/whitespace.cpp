#include <catch2/catch_test_macros.hpp>

#include "bython/lexy_frontend/grammar.hpp"
#include "unwrap.hpp"

namespace g = bython::grammar;
namespace ast = bython::ast;

TEST_CASE("Reach EOF")
{
  unwrap_grammar<g::var_or_call, ast::variable>("variable");
}

TEST_CASE("Consume Whitespace until EOF")
{
  unwrap_grammar<g::var_or_call, ast::variable>("variable       ");
}
