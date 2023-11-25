#include "bython/type_system/subtyping.hpp"

#include <catch2/catch_test_macros.hpp>

#include "bython/type_system.hpp"
#include "bython/type_system/builtin.hpp"
#include "bython/type_system/environment.hpp"

namespace ts = bython::type_system;

TEST_CASE("Subtyping of Builtins", "[Type System]")
{
  auto const environment = ts::environment::initialise_with_builtins();

  SECTION("Builtin Types are Known")
  {
    // signed
    REQUIRE(environment.lookup("i8"));
    REQUIRE(environment.lookup("i16"));
    REQUIRE(environment.lookup("i32"));
    REQUIRE(environment.lookup("i64"));

    // unsigned
    REQUIRE(environment.lookup("u8"));
    REQUIRE(environment.lookup("u16"));
    REQUIRE(environment.lookup("u32"));
    REQUIRE(environment.lookup("u64"));

    // floating point
    REQUIRE(environment.lookup("f32"));
    REQUIRE(environment.lookup("f64"));
  }

  SECTION("Identical Types correspond to the Identity Rule")
  {
    REQUIRE(environment.try_subtype(ts::sint {8}, ts::sint {8}) == ts::subtyping_rule::identity);
    REQUIRE(environment.try_subtype(ts::uint {16}, ts::uint {16}) == ts::subtyping_rule::identity);
    REQUIRE(environment.try_subtype(ts::single_fp {}, ts::single_fp {})
            == ts::subtyping_rule::identity);
    REQUIRE(environment.try_subtype(ts::double_fp {}, ts::double_fp {})
            == ts::subtyping_rule::identity);
  }

  SECTION("Smaller Signed Integers are Subtypes of Larger Signed Integers")
  {
    REQUIRE(environment.try_subtype(ts::sint {8}, ts::sint {16})
            == ts::subtyping_rule::sint_promotion);
    REQUIRE(environment.try_subtype(ts::sint {16}, ts::sint {32})
            == ts::subtyping_rule::sint_promotion);
    REQUIRE(environment.try_subtype(ts::sint {32}, ts::sint {64})
            == ts::subtyping_rule::sint_promotion);
  }

  SECTION("Smaller Unsigned Integers are Subtypes of Larger Unsigned Integers")
  {
    REQUIRE(environment.try_subtype(ts::uint {8}, ts::uint {16})
            == ts::subtyping_rule::uint_promotion);
    REQUIRE(environment.try_subtype(ts::uint {16}, ts::uint {32})
            == ts::subtyping_rule::uint_promotion);
    REQUIRE(environment.try_subtype(ts::uint {32}, ts::uint {64})
            == ts::subtyping_rule::uint_promotion);
  }

  SECTION("Larger Signed Integers are not Subtypes of Smaller Signed Integers")
  {
    REQUIRE_FALSE(environment.try_subtype(ts::sint {16}, ts::sint {8}));
    REQUIRE_FALSE(environment.try_subtype(ts::sint {32}, ts::sint {16}));
    REQUIRE_FALSE(environment.try_subtype(ts::sint {64}, ts::sint {32}));
  }

  SECTION("Larger Unsigned Integers are not Subtypes of Smaller Unsigned Integers")
  {
    REQUIRE_FALSE(environment.try_subtype(ts::uint {16}, ts::uint {8}));
    REQUIRE_FALSE(environment.try_subtype(ts::uint {32}, ts::uint {16}));
    REQUIRE_FALSE(environment.try_subtype(ts::uint {64}, ts::uint {32}));
  }
}