/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "../catch2.hpp"
#include <bill/dd/cudd_zdd.hpp>

TEST_CASE("CUDD ZDD choose operator", "[cudd]")
{
  std::ostringstream os;
  cudd::cudd_zdd zdd( 3 );

  auto zdd_0 = zdd.elementary(0);
  auto zdd_1 = zdd.elementary(1);
  auto zdd_2 = zdd.elementary(2);
  auto zdd_union = zdd.union_(zdd_0, zdd_1);
  zdd_union = zdd.union_(zdd_union, zdd_2);
  
  auto zdd_choose = zdd.choose(zdd_union, 2);
  zdd.print_sets(zdd_choose, os);
  CHECK(os.str() == "{ 1, 2 }\n{ 0, 2 }\n{ 0, 1 }\n");
}

TEST_CASE("CUDD ZDD join operator (*)", "[cudd]")
{
  std::ostringstream os;
  cudd::cudd_zdd zdd(7);

  auto zdd_0 = zdd.elementary(0);
  auto zdd_1 = zdd.elementary(1);
  auto zdd_2 = zdd.elementary(2);
  auto zdd_3 = zdd.elementary(3);
  auto zdd_4 = zdd.elementary(4);
  auto zdd_5 = zdd.elementary(5);
  auto zdd_6 = zdd.elementary(6);

  SECTION("Check 0 * 0 == 0")
  {
    auto result = zdd.join(zdd.bottom(), zdd.bottom());
    CHECK(result == zdd.bottom());
  }
  SECTION("Check {0} * 0 == 0 * {0} == 0")
  {
    auto result_0b = zdd.join(zdd_0, zdd.bottom());
    auto result_b0 = zdd.join(zdd.bottom(), zdd_0);
    CHECK(result_0b == result_b0);
    CHECK(result_0b == zdd.bottom());
  }
  SECTION("Check {0} * {} == {} * {0} == {0}")
  {
    auto result_0t = zdd.join(zdd_0, zdd.top());
    auto result_t0 = zdd.join(zdd.top(), zdd_0);
    CHECK(result_0t == result_t0);
    CHECK(result_0t == zdd_0);
  }
  SECTION("Check {0} * {1} == {1} * {0} == {0,1}")
  {
    auto result_01 = zdd.join(zdd_0, zdd_1);
    auto result_10 = zdd.join(zdd_1, zdd_0);
    CHECK(result_01 == result_10);
    CHECK(zdd.count_sets(result_01) == 1u);
    zdd.print_sets(result_01, os);
    CHECK(os.str() == "{ 0, 1 }\n");
  }
  SECTION("Check ({0} * {1}) * {2} == {0} * ({1} * {2}) == ({0} * {2}) * {1} == {0,1,2}")
  {
    auto result_01_2 = zdd.join(zdd.join(zdd_0, zdd_1), zdd_2);
    auto result_0_12 = zdd.join(zdd_0, zdd.join(zdd_1, zdd_2));
    auto result_02_1 = zdd.join(zdd.join(zdd_0, zdd_2), zdd_1);
    CHECK(result_01_2 == result_0_12);
    CHECK(result_01_2 == result_02_1);
    CHECK(zdd.count_sets(result_01_2) == 1u);
    zdd.print_sets(result_01_2, os);
    CHECK(os.str() == "{ 0, 1, 2 }\n");
  }
  // Partial sets
  auto zdd_123 = zdd.join(zdd_1, zdd.join(zdd_2, zdd_3));
  auto zdd_34  = zdd.join(zdd_3, zdd_4);
  auto zdd_023 = zdd.join(zdd_0, zdd.join(zdd_2, zdd_3));

  // X = {{1,2,3}, {3,4}, {5}}
  auto zdd_x = zdd.union_(zdd_123, zdd.union_(zdd_34, zdd_5));
  SECTION("Check X")
  {
    CHECK(zdd.count_sets(zdd_x) == 3u);
    zdd.print_sets(zdd_x, os);
    CHECK(os.str() == "{ 5 }\n{ 3, 4 }\n{ 1, 2, 3 }\n");
  }
  // Y = {{0,2,3}, {3,4}, {6}}
  auto zdd_y = zdd.union_(zdd_023, zdd.union_(zdd_34, zdd_6));
  SECTION("Check Y")
  {
    CHECK(zdd.count_sets(zdd_y) == 3u);
    zdd.print_sets(zdd_y, os);
    CHECK(os.str() == "{ 6 }\n{ 3, 4 }\n{ 0, 2, 3 }\n");
  }

  SECTION("Check X * 0 == 0 * X == 0")
  {
    auto result_Xb = zdd.join(zdd_x, zdd.bottom());
    auto result_bX = zdd.join(zdd.bottom(), zdd_x);
    CHECK(result_Xb == result_bX);
    CHECK(result_Xb == zdd.bottom());
  }
  SECTION("Check X * {} == {} * X == X")
  {
    auto result_Xt = zdd.join(zdd_x, zdd.top());
    auto result_tX = zdd.join(zdd.top(), zdd_x);
    CHECK(result_Xt == result_tX);
    CHECK(result_Xt == zdd_x);
  }
  SECTION("Check X * Y == Y * X")
  {
    auto result_xy = zdd.join(zdd_x, zdd_y);
    auto result_yx = zdd.join(zdd_y, zdd_x);
    CHECK(result_xy == result_yx);
    CHECK(zdd.count_sets(result_xy) == 9u);
    zdd.print_sets(result_xy, os);
    CHECK(os.str() == "{ 5, 6 }\n{ 3, 4 }\n{ 3, 4, 6 }\n{ 3, 4, 5 }\n{ 1, 2, 3, 6 }\n"
                      "{ 1, 2, 3, 4 }\n{ 0, 2, 3, 5 }\n{ 0, 2, 3, 4 }\n{ 0, 1, 2, 3 }\n");
  }
}

