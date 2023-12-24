#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "bython/ast.hpp"
#include "bython/ast/expression.hpp"
#include "bython/frontend/frontend.hpp"
#include "bython/frontend/lexy.hpp"
#include "bython/type_system.hpp"

namespace ast = bython::ast;
namespace ts = bython::type_system;
namespace p = bython::parser;

namespace
{
auto const i8_max = std::numeric_limits<std::int8_t>::max();
auto const i16_max = std::numeric_limits<std::int16_t>::max();
auto const i32_max = std::numeric_limits<std::int32_t>::max();
auto const i64_max = std::numeric_limits<std::int64_t>::max();

auto const u8_max = std::numeric_limits<std::uint8_t>::max();
auto const u16_max = std::numeric_limits<std::uint16_t>::max();
auto const u32_max = std::numeric_limits<std::uint32_t>::max();
auto const u64_max = std::numeric_limits<std::uint64_t>::max();

auto parse_expression(std::string_view code)
    -> std::tuple<std::unique_ptr<p::parse_metadata>, std::shared_ptr<ast::expression>>
{
  CAPTURE(code);

  static auto parser = p::lexy_code_frontend {};
  auto result = parser.parse_expression(code);
  if (result.has_error()) {
    auto error = std::move(result).error();
    INFO(error);
    FAIL();
  }

  auto [metadata, node] = std::move(result).value();

  auto node_as_sp = std::shared_ptr<ast::node>(std::move(node));
  auto expr_as_sp = std::dynamic_pointer_cast<ast::expression>(node_as_sp);
  if (!expr_as_sp) {
    metadata->report_error(
        *node_as_sp, p::frontend_error_report {.message = "Code must be parsed as an expression"});
    FAIL();
  }

  return std::make_tuple(std::move(metadata), std::move(expr_as_sp));
}
}  // namespace

TEST_CASE("Primitives", "[Inference]")
{
  auto env = ts::environment::initialise_with_builtins();

  SECTION("u8")
  {
    auto [_, expr] = parse_expression(std::to_string(u8_max));
    auto expr_type = env.get_type(*expr);

    REQUIRE(expr_type);
    REQUIRE(expr_type == env.lookup_type("u8"));
  }

  SECTION("u16")
  {
    auto [_, expr] = parse_expression(std::to_string(u16_max));
    auto expr_type = env.get_type(*expr);

    REQUIRE(expr_type);
    REQUIRE(expr_type == env.lookup_type("u16"));
  }

  SECTION("u32")
  {
    auto [_, expr] = parse_expression(std::to_string(u32_max));
    auto expr_type = env.get_type(*expr);

    REQUIRE(expr_type);
    REQUIRE(expr_type == env.lookup_type("u32"));
  }

  SECTION("u64")
  {
    auto [_, expr] = parse_expression(std::to_string(u64_max));
    auto expr_type = env.get_type(*expr);

    REQUIRE(expr_type);
    REQUIRE(expr_type == env.lookup_type("u64"));
  }
}

TEST_CASE("Binary Operations", "[Inference]")
{
  // same signage       => +, -, /, *, % all promote to the larger operand if identical signage
  // different signage  => promote to unsigned if same rank, otherwise promote to the larger one

  auto env = ts::environment::initialise_with_builtins();

  auto operators = GENERATE(ast::binop_tag::plus,
                            ast::binop_tag::minus,
                            ast::binop_tag::multiply,
                            ast::binop_tag::divide,
                            ast::binop_tag::modulo);

  SECTION("Signed")
  {
    auto i8i64 = ast::binary_operation(std::make_unique<ast::signed_integer>(i8_max),
                                       operators,
                                       std::make_unique<ast::signed_integer>(i64_max));
    auto inferred_i8i64 = env.get_type(i8i64);
    REQUIRE(inferred_i8i64);
    REQUIRE(*inferred_i8i64 == *env.lookup_type("i64"));

    auto i64i8 = ast::binary_operation(std::make_unique<ast::signed_integer>(i64_max),
                                       operators,
                                       std::make_unique<ast::signed_integer>(i8_max));
    auto inferred_i64i8 = env.get_type(i64i8);
    REQUIRE(inferred_i64i8);
    REQUIRE(*inferred_i64i8 == *env.lookup_type("i64"));
  }

  SECTION("Unsigned")
  {
    auto u8u64 = ast::binary_operation(std::make_unique<ast::unsigned_integer>(u8_max),
                                       operators,
                                       std::make_unique<ast::unsigned_integer>(u64_max));
    auto inferred_u8u64 = env.get_type(u8u64);
    REQUIRE(inferred_u8u64);
    REQUIRE(*inferred_u8u64 == *env.lookup_type("u64"));

    auto u64u8 = ast::binary_operation(std::make_unique<ast::unsigned_integer>(u64_max),
                                       operators,
                                       std::make_unique<ast::unsigned_integer>(u8_max));
    auto inferred_u64u8 = env.get_type(u64u8);
    REQUIRE(inferred_u64u8);
    REQUIRE(*inferred_u64u8 == *env.lookup_type("u64"));
  }

  SECTION("Mixed different size")
  {
    auto u8i64 = ast::binary_operation(std::make_unique<ast::unsigned_integer>(u8_max),
                                       operators,
                                       std::make_unique<ast::signed_integer>(i64_max));
    auto inferred_u8i64 = env.get_type(u8i64);
    REQUIRE(inferred_u8i64);
    REQUIRE(*inferred_u8i64 == *env.lookup_type("i64"));

    auto u64i8 = ast::binary_operation(std::make_unique<ast::unsigned_integer>(u64_max),
                                       operators,
                                       std::make_unique<ast::signed_integer>(i8_max));
    auto inferred_u64i8 = env.get_type(u64i8);
    REQUIRE(inferred_u64i8);
    REQUIRE(*inferred_u64i8 == *env.lookup_type("u64"));
  }
}