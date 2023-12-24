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
    // boolean
    REQUIRE(environment.lookup_type("bool"));

    // signed
    REQUIRE(environment.lookup_type("i8"));
    REQUIRE(environment.lookup_type("i16"));
    REQUIRE(environment.lookup_type("i32"));
    REQUIRE(environment.lookup_type("i64"));

    // unsigned
    REQUIRE(environment.lookup_type("u8"));
    REQUIRE(environment.lookup_type("u16"));
    REQUIRE(environment.lookup_type("u32"));
    REQUIRE(environment.lookup_type("u64"));

    // floating point
    REQUIRE(environment.lookup_type("f32"));
    REQUIRE(environment.lookup_type("f64"));
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

  SECTION("Signed Integer Promotion")
  {
    REQUIRE(environment.try_subtype(ts::sint {8}, ts::sint {16})
            == ts::subtyping_rule::sint_promotion);
    REQUIRE(environment.try_subtype(ts::sint {16}, ts::sint {32})
            == ts::subtyping_rule::sint_promotion);
    REQUIRE(environment.try_subtype(ts::sint {32}, ts::sint {64})
            == ts::subtyping_rule::sint_promotion);
  }

  SECTION("Unsigned Integer Promotion")
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

  SECTION("Unsigned Integer to Single")
  {
    REQUIRE(environment.try_subtype(ts::uint {8}, ts::single_fp {})
            == ts::subtyping_rule::uint_to_single);
    REQUIRE(environment.try_subtype(ts::uint {16}, ts::single_fp {})
            == ts::subtyping_rule::uint_to_single);
    REQUIRE(environment.try_subtype(ts::uint {32}, ts::single_fp {})
            == ts::subtyping_rule::uint_to_single);
    REQUIRE(environment.try_subtype(ts::uint {64}, ts::single_fp {})
            == ts::subtyping_rule::uint_to_single);
  }

  SECTION("Unsigned Integer to Double")
  {
    REQUIRE(environment.try_subtype(ts::uint {8}, ts::double_fp {})
            == ts::subtyping_rule::uint_to_double);
    REQUIRE(environment.try_subtype(ts::uint {16}, ts::double_fp {})
            == ts::subtyping_rule::uint_to_double);
    REQUIRE(environment.try_subtype(ts::uint {32}, ts::double_fp {})
            == ts::subtyping_rule::uint_to_double);
    REQUIRE(environment.try_subtype(ts::uint {64}, ts::double_fp {})
            == ts::subtyping_rule::uint_to_double);
  }

  SECTION("Signed Integer to Single")
  {
    REQUIRE(environment.try_subtype(ts::sint {8}, ts::single_fp {})
            == ts::subtyping_rule::sint_to_single);
    REQUIRE(environment.try_subtype(ts::sint {16}, ts::single_fp {})
            == ts::subtyping_rule::sint_to_single);
    REQUIRE(environment.try_subtype(ts::sint {32}, ts::single_fp {})
            == ts::subtyping_rule::sint_to_single);
    REQUIRE(environment.try_subtype(ts::sint {64}, ts::single_fp {})
            == ts::subtyping_rule::sint_to_single);
  }

  SECTION("Signed Integer to Double")
  {
    REQUIRE(environment.try_subtype(ts::sint {8}, ts::double_fp {})
            == ts::subtyping_rule::sint_to_double);
    REQUIRE(environment.try_subtype(ts::sint {16}, ts::double_fp {})
            == ts::subtyping_rule::sint_to_double);
    REQUIRE(environment.try_subtype(ts::sint {32}, ts::double_fp {})
            == ts::subtyping_rule::sint_to_double);
    REQUIRE(environment.try_subtype(ts::sint {64}, ts::double_fp {})
            == ts::subtyping_rule::sint_to_double);
  }

  SECTION("Floating Point Promotion")
  {
    REQUIRE(environment.try_subtype(ts::single_fp {}, ts::double_fp {})
            == ts::subtyping_rule::single_to_double);
    REQUIRE_FALSE(environment.try_subtype(ts::double_fp {}, ts::single_fp {})
                  == ts::subtyping_rule::single_to_double);
  }

  SECTION("Float to Bool")
  {
    REQUIRE(environment.try_subtype(ts::single_fp {}, ts::boolean {})
            == ts::subtyping_rule::numeric_to_bool);
    REQUIRE(environment.try_subtype(ts::double_fp {}, ts::boolean {})
            == ts::subtyping_rule::numeric_to_bool);
  }

  SECTION("Bool to Float")
  {
    REQUIRE(environment.try_subtype(ts::boolean {}, ts::single_fp {})
            == ts::subtyping_rule::bool_fp_prom);
    REQUIRE(environment.try_subtype(ts::boolean {}, ts::double_fp {})
            == ts::subtyping_rule::bool_fp_prom);
  }

  SECTION("Int to Bool")
  {
    REQUIRE(environment.try_subtype(ts::sint {8}, ts::boolean {}) == ts::subtyping_rule::numeric_to_bool);
    REQUIRE(environment.try_subtype(ts::uint {64}, ts::boolean {}) == ts::subtyping_rule::numeric_to_bool);
  }

  SECTION("Bool to Int")
  {
    REQUIRE(environment.try_subtype(ts::boolean {}, ts::uint {8})
            == ts::subtyping_rule::bool_int_prom);
    REQUIRE(environment.try_subtype(ts::boolean {}, ts::sint {16})
            == ts::subtyping_rule::bool_int_prom);
  }
}