#include <iostream>

#include <bython/ast.hpp>
#include <bython/matching.hpp>
#include <bython/codegen/executors.hpp>
#include <catch2/catch_test_macros.hpp>

namespace c = bython::codegen;

TEST_CASE("Binary Operation", "[LLVM Codegen]")
{
  auto interpreter = c::interpreter {};

  interpreter.repl("var x = 5");
  // TODO: Placeholder
  // REQUIRE(interpreter.retrieve_int_state("x") == 5);
}