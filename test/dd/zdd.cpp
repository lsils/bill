/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "../catch2.hpp"

#include <bill/dd/zdd.hpp>
#include <sstream>

// TODO: Improve test case for choose
// TODO: Implement test case for nonsubsets
// TODO: Implement test case for nonsupersets
// TODO: Implement test case which uses garbage collection

TEST_CASE("ZDD Constructor", "[zdd]")
{
	using namespace bill;
	SECTION("An empty ZDD")
	{
		zdd_base zdd(0);
		CHECK(zdd.num_nodes() == 0u);
		CHECK(zdd.bottom() == 0u);
		CHECK(zdd.top() == 1u);
		CHECK(zdd.count_nodes(0) == 0u);
		CHECK(zdd.count_nodes(1) == 0u);
		CHECK(zdd.count_sets(0) == 0u);
		CHECK(zdd.count_sets(1) == 1u);
	}
	SECTION("An ZDD base with 1 variable")
	{
		zdd_base zdd(1);
		auto zdd_0 = zdd.elementary(0);
		CHECK(zdd.num_nodes() == 2u);
		CHECK(zdd_0 == 2u);
		CHECK(zdd.count_nodes(zdd_0) == 1u);
		CHECK(zdd.count_sets(zdd_0) == 1u);
	}
	SECTION("An ZDD base with 4096 variable")
	{
		zdd_base zdd(4095);
		CHECK(zdd.num_nodes() == (4095u << 1));
		CHECK(zdd.elementary(4094u) == 4096);
	}
}

TEST_CASE("ZDD choose operator", "[zdd]")
{
	using namespace bill;
	std::ostringstream os;
	zdd_base zdd(3);
	auto zdd_0 = zdd.elementary(0);
	auto zdd_1 = zdd.elementary(1);
	auto zdd_2 = zdd.elementary(2);
	auto zdd_union = zdd.union_(zdd_0, zdd_1);
	zdd_union = zdd.union_(zdd_union, zdd_2);
	
	auto zdd_choose = zdd.choose(zdd_union, 2);
	zdd.print_sets(zdd_choose, os);
	CHECK(os.str() == "{ 1, 2 }\n{ 0, 2 }\n{ 0, 1 }\n");
}

TEST_CASE("ZDD difference operator (-)", "[zdd]")
{
	using namespace bill;
	std::ostringstream os;
	zdd_base zdd(7);
	auto zdd_0 = zdd.elementary(0);
	auto zdd_1 = zdd.elementary(1);
	auto zdd_2 = zdd.elementary(2);
	auto zdd_3 = zdd.elementary(3);
	auto zdd_4 = zdd.elementary(4);
	auto zdd_5 = zdd.elementary(5);
	auto zdd_6 = zdd.elementary(6);
	
	SECTION("Check 0 - 0 == 0")
	{
		auto result = zdd.difference(zdd.bottom(), zdd.bottom());
		CHECK(result == zdd.bottom());
	}
	SECTION("Check {} - {} == 0")
	{
		auto result = zdd.difference(zdd.top(), zdd.top());
		CHECK(result == zdd.bottom());
	}
	SECTION("Check ({} - 0 == {}) != (0 - {} == 0)")
	{
		auto result_tb = zdd.difference(zdd.top(), zdd.bottom());
		auto result_bt = zdd.difference(zdd.bottom(), zdd.top());
		CHECK(result_tb != result_bt);
		CHECK(result_tb == zdd.top());
		CHECK(result_bt == zdd.bottom());
	}
	SECTION("Check ({0} - 0 == {0}) != (0 - {0} == 0)")
	{
		auto result_0b = zdd.difference(zdd_0, zdd.bottom());
		auto result_b0 = zdd.difference(zdd.bottom(), zdd_0);
		CHECK(result_0b != result_b0);
		CHECK(result_0b == zdd_0);
		CHECK(result_b0 == zdd.bottom());
	}
	SECTION("Check ({0} - {} == {0}) != ({} - {0} == {})")
	{
		auto result_0t = zdd.difference(zdd_0, zdd.top());
		auto result_t0 = zdd.difference(zdd.top(), zdd_0);
		CHECK(result_0t != result_t0);
		CHECK(result_0t == zdd_0);
		CHECK(result_t0 == zdd.top());
	}
	SECTION("Check {0} - {0} == 0")
	{
		auto result = zdd.difference(zdd_0, zdd_0);
		CHECK(result == zdd.bottom());
	}
	// Partial sets
	auto zdd_123 = zdd.join(zdd_1, zdd.join(zdd_2, zdd_3));
	auto zdd_34  = zdd.join(zdd_3, zdd_4);
	auto zdd_023 = zdd.join(zdd_0, zdd.join(zdd_2, zdd_3));

	// X = {{1,2,3}, {3,4}, {5}}
	auto zdd_x = zdd.union_(zdd_123, zdd.union_(zdd_34, zdd_5));
	SECTION("Check (X - 0 == X) != (0 - X == 0)")
	{
		auto result_xb = zdd.difference(zdd_x, zdd.bottom());
		auto result_bx = zdd.difference(zdd.bottom(), zdd_x);
		CHECK(result_xb != result_bx);
		CHECK(result_xb == zdd_x);
		CHECK(result_bx == zdd.bottom());
	}
	SECTION("Check (X - {} == X) != ({} - X == {})")
	{
		auto result_xt = zdd.difference(zdd_x, zdd.top());
		auto result_tx = zdd.difference(zdd.top(), zdd_x);
		CHECK(result_xt != result_tx);
		CHECK(result_xt == zdd_x);
		CHECK(result_tx == zdd.top());
	}
	SECTION("Check X - X == 0")
	{
		auto result = zdd.difference(zdd_x, zdd_x);
		CHECK(result == zdd.bottom());
	}

	// Y = {{0,2,3}, {3,4}, {6}}
	auto zdd_y = zdd.union_(zdd_023, zdd.union_(zdd_34, zdd_6));
	SECTION("Check (X - Y == {{5},{1,2,3}}) != (Y - X == {{6},{0,2,3}})")
	{
		auto result_xy = zdd.difference(zdd_x, zdd_y);
		auto result_yx = zdd.difference(zdd_y, zdd_x);
		CHECK(result_xy != result_yx);
		zdd.print_sets(result_xy, os);
		CHECK(os.str() == "{ 5 }\n{ 1, 2, 3 }\n");
		os.str("");
		zdd.print_sets(result_yx, os);
		CHECK(os.str() == "{ 6 }\n{ 0, 2, 3 }\n");
	}
}

TEST_CASE("ZDD intersection operator (&)", "[zdd]")
{
	using namespace bill;
	std::ostringstream os;
	zdd_base zdd(7);
	auto zdd_0 = zdd.elementary(0);
	auto zdd_1 = zdd.elementary(1);
	auto zdd_2 = zdd.elementary(2);
	auto zdd_3 = zdd.elementary(3);
	auto zdd_4 = zdd.elementary(4);
	auto zdd_5 = zdd.elementary(5);
	auto zdd_6 = zdd.elementary(6);
	
	SECTION("Check 0 & 0 == 0")
	{
		auto result = zdd.intersection(zdd.bottom(), zdd.bottom());
		CHECK(result == zdd.bottom());
	}
	SECTION("Check {0} & {1} = 0")
	{
		auto result = zdd.intersection(zdd_0, zdd_1);
		CHECK(zdd.count_sets(result) == 0u);
		zdd.print_sets(result, os);
		CHECK(os.str() == "");
	}

	// Partial sets
	auto zdd_123 = zdd.join(zdd_1, zdd.join(zdd_2, zdd_3));
	auto zdd_34  = zdd.join(zdd_3, zdd_4);
	auto zdd_023 = zdd.join(zdd_0, zdd.join(zdd_2, zdd_3));

	// X = {{1,2,3}, {3,4}, {5}}
	auto zdd_x = zdd.union_(zdd_123, zdd.union_(zdd_34, zdd_5));
	// Y = {{0,2,3}, {3,4}, {6}}
	auto zdd_y = zdd.union_(zdd_023, zdd.union_(zdd_34, zdd_6));
	// Z = {{3,4}, {5}, {6}}
	auto zdd_z = zdd.union_(zdd_34, zdd.union_(zdd_5, zdd_6));

	SECTION("Check X & Y == Y & X== {{3,4}}")
	{
		auto result_xy = zdd.intersection(zdd_x, zdd_y);
		auto result_yx = zdd.intersection(zdd_y, zdd_x);
		CHECK(result_xy == result_yx);
		CHECK(zdd.count_sets(result_xy) == 1u);
		zdd.print_sets(result_xy, os);
		CHECK(os.str() == "{ 3, 4 }\n");
	}
	SECTION("Associative. Check: (X & Y) & Z == X & (Y & Z) == (X & Z) & Y = {{3,4}}")
	{
		auto result_xy_c = zdd.intersection(zdd.intersection(zdd_x, zdd_y), zdd_z);
		auto result_a_bc = zdd.intersection(zdd_x, zdd.intersection(zdd_y, zdd_z));
		auto result_ac_b = zdd.intersection(zdd.intersection(zdd_x, zdd_z), zdd_y);
		CHECK(result_xy_c == result_a_bc);
		CHECK(result_xy_c == result_ac_b);
		CHECK(zdd.count_sets(result_xy_c) == 1u);
		zdd.print_sets(result_xy_c, os);
		CHECK(os.str() == "{ 3, 4 }\n");
	}
	SECTION("Idempotence. Check: X & X == X")
	{
		auto result = zdd.intersection(zdd_x, zdd_x);
		CHECK(result == zdd_x);
		CHECK(zdd.count_sets(result) == 3u);
		zdd.print_sets(result, os);
		CHECK(os.str() == "{ 5 }\n{ 3, 4 }\n{ 1, 2, 3 }\n");
	}
	SECTION("Domination. Check: X & 0 == 0 & X == 0") // Where 0 is the empty set
	{
		auto result_ab = zdd.intersection(zdd_x, zdd.bottom());
		auto result_ba = zdd.intersection(zdd.bottom(), zdd_x);
		CHECK(result_ab == result_ba);
		CHECK(result_ab == zdd.bottom());
	}
	SECTION("Check 0 & 0 = 0")
	{
		auto result = zdd.intersection(zdd.bottom(), zdd.bottom());
		CHECK(result == zdd.bottom());
	}
	SECTION("Identity. Check: X & 1 == 1 & X == X") // Where 1 is tautology (universe)
	{
		auto result_a1 = zdd.intersection(zdd_x, zdd.tautology());
		auto result_1a = zdd.intersection(zdd.tautology(), zdd_x);
		CHECK(result_a1 == result_1a);
		CHECK(result_a1 == zdd_x);
	}
}

TEST_CASE("ZDD join operator (*)", "[zdd]")
{
	using namespace bill;
	std::ostringstream os;
	zdd_base zdd(7);
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

TEST_CASE("ZDD meet operator (@)", "[zdd]")
{
	using namespace bill;
	std::ostringstream os;
	zdd_base zdd(7);
	auto zdd_0 = zdd.elementary(0);
	auto zdd_1 = zdd.elementary(1);
	auto zdd_2 = zdd.elementary(2);
	auto zdd_3 = zdd.elementary(3);
	auto zdd_4 = zdd.elementary(4);
	auto zdd_5 = zdd.elementary(5);
	auto zdd_6 = zdd.elementary(6);

	SECTION("Check 0 @ 0 == 0")
	{
		auto result = zdd.meet(zdd.bottom(), zdd.bottom());
		CHECK(result == zdd.bottom());
	}
	SECTION("Check {} @ {} == {}")
	{
		auto result = zdd.meet(zdd.top(), zdd.top());
		CHECK(result == zdd.top());
	}
	SECTION("Check 0 @ {} == {} @ 0 == 0")
	{
		auto result_bt = zdd.meet(zdd.bottom(), zdd.top());
		auto result_tb = zdd.meet(zdd.top(), zdd.bottom());
		CHECK(result_bt == result_tb);
		CHECK(result_bt == zdd.bottom());
	}
	SECTION("Check {0} @ 0 == 0 @ {0} == 0")
	{
		auto result_0b = zdd.meet(zdd_0, zdd.bottom());
		auto result_b0 = zdd.meet(zdd.bottom(), zdd_0);
		CHECK(result_0b == result_b0);
		CHECK(result_0b == zdd.bottom());
	}
	SECTION("Check {0} @ {} == {} @ {0} == {}")
	{
		auto result_0t = zdd.meet(zdd_0, zdd.top());
		auto result_t0 = zdd.meet(zdd.top(), zdd_0);
		CHECK(result_0t == result_t0);
		CHECK(result_0t == zdd.top());
	}
	SECTION("Check {5} @ {5} = {5}")
	{
		auto result = zdd.meet(zdd_5, zdd_5);
		CHECK(result == zdd_5);
	}
	SECTION("Check {5} @ {6} == {6} @ {5} == {}")
	{
		auto result_56 = zdd.meet(zdd_5, zdd_6);
		auto result_65 = zdd.meet(zdd_6, zdd_5);
		CHECK(result_56 == result_65);
		CHECK(result_56 == zdd.top());
	}

	// Partial sets
	auto zdd_123 = zdd.join(zdd_1, zdd.join(zdd_2, zdd_3));
	auto zdd_34  = zdd.join(zdd_3, zdd_4);
	auto zdd_023 = zdd.join(zdd_0, zdd.join(zdd_2, zdd_3));

	// X = {{1,2,3}, {3,4}, {5}}
	auto zdd_x = zdd.union_(zdd_123, zdd.union_(zdd_34, zdd_5));
	// Y = {{0,2,3}, {3,4}, {6}}
	auto zdd_y = zdd.union_(zdd_023, zdd.union_(zdd_34, zdd_6));

	SECTION("Check X * 0 == 0 * X == 0")
	{
		auto result_Xb = zdd.meet(zdd_x, zdd.bottom());
		auto result_bX = zdd.meet(zdd.bottom(), zdd_x);
		CHECK(result_Xb == result_bX);
		CHECK(result_Xb == zdd.bottom());
	}
	SECTION("Check X * {} == {} * X == {}")
	{
		auto result_Xt = zdd.meet(zdd_x, zdd.top());
		auto result_tX = zdd.meet(zdd.top(), zdd_x);
		CHECK(result_Xt == result_tX);
		CHECK(result_Xt == zdd.top());
	}
	SECTION("Check X @ {6} == {6} @ X == 0")
	{
		auto result_x6 = zdd.meet(zdd_x, zdd_6);
		auto result_6x = zdd.meet(zdd_6, zdd_x);
		CHECK(result_x6 == result_6x);
		CHECK(result_x6 == zdd.top());
	}
	SECTION("Check X @ Y == Y @ X")
	{
		auto result_xy = zdd.meet(zdd_x, zdd_y);
		auto result_yx = zdd.meet(zdd_y, zdd_x);
		CHECK(result_xy == result_yx);
		CHECK(zdd.count_sets(result_xy) == 4u);
		zdd.print_sets(result_xy, os);
		CHECK(os.str() == "{  }\n{ 3 }\n{ 3, 4 }\n{ 2, 3 }\n");
	}
}

TEST_CASE("ZDD union operator (|)", "[zdd]")
{
	using namespace bill;
	std::ostringstream os;
	zdd_base zdd(7);
	auto zdd_0 = zdd.elementary(0);
	auto zdd_1 = zdd.elementary(1);
	auto zdd_2 = zdd.elementary(2);
	auto zdd_3 = zdd.elementary(3);
	auto zdd_4 = zdd.elementary(4);
	auto zdd_5 = zdd.elementary(5);
	auto zdd_6 = zdd.elementary(6);

	SECTION("Check 0 | 0 == 0")
	{
		auto result = zdd.union_(zdd.bottom(), zdd.bottom());
		CHECK(result == zdd.bottom());
	}
	
	// Partial sets
	auto zdd_123 = zdd.join(zdd_1, zdd.join(zdd_2, zdd_3));
	auto zdd_34  = zdd.join(zdd_3, zdd_4);
	auto zdd_023 = zdd.join(zdd_0, zdd.join(zdd_2, zdd_3));

	// X = {{1,2,3}, {3,4}, {5}}
	auto zdd_x = zdd.union_(zdd_123, zdd.union_(zdd_34, zdd_5));
	// Y = {{0,2,3}, {3,4}, {6}}
	auto zdd_y = zdd.union_(zdd_023, zdd.union_(zdd_34, zdd_6));
	// Z = {{3,4}, {5}, {6}}
	auto zdd_z = zdd.union_(zdd_34, zdd.union_(zdd_5, zdd_6));

	SECTION("Check {0} | {1} = {{0}, {1}}")
	{
		auto result = zdd.union_(zdd_0, zdd_1);
		CHECK(zdd.count_sets(result) == 2u);
		zdd.print_sets(result, os);
		CHECK(os.str() == "{ 1 }\n{ 0 }\n");
	}
	SECTION("Check X | Y == Y | X == {{6}, {5}, {3,4}, {1,2,3}, {0,2,3}}")
	{
		auto result_xy = zdd.union_(zdd_x, zdd_y);
		auto result_yx = zdd.union_(zdd_y, zdd_x);
		CHECK(result_xy == result_yx);
		CHECK(zdd.count_sets(result_xy) == 5u);
		zdd.print_sets(result_xy, os);
		CHECK(os.str() == "{ 6 }\n{ 5 }\n{ 3, 4 }\n{ 1, 2, 3 }\n{ 0, 2, 3 }\n");
	}
	SECTION("Associative. Check: (X | Y) | Z == X | (Y | Z) == (X | Z) | Y = {{3,4}}")
	{
		auto result_xy_c = zdd.union_(zdd.union_(zdd_x, zdd_y), zdd_z);
		auto result_a_bc = zdd.union_(zdd_x, zdd.union_(zdd_y, zdd_z));
		auto result_ac_b = zdd.union_(zdd.union_(zdd_x, zdd_z), zdd_y);
		CHECK(result_xy_c == result_a_bc);
		CHECK(result_xy_c == result_ac_b);
		CHECK(zdd.count_sets(result_xy_c) == 5u);
		zdd.print_sets(result_xy_c, os);
		CHECK(os.str() == "{ 6 }\n{ 5 }\n{ 3, 4 }\n{ 1, 2, 3 }\n{ 0, 2, 3 }\n");
	}
	SECTION("Idempotence. Check: X | X == X")
	{
		auto result = zdd.union_(zdd_x, zdd_x);
		CHECK(result == zdd_x);
	}
	SECTION("Domination. Check: X | 1 == 1 | X == 1") // Where 1 is tautology (universe)
	{
		auto result_x1 = zdd.union_(zdd_x, zdd.tautology());
		auto result_1x = zdd.union_(zdd.tautology(), zdd_x);
		CHECK(result_x1 == result_1x);
		CHECK(result_x1 == zdd.tautology());
	}
	SECTION("Identity. Check: X | 0 == 0 | X == X") // Where 0 is the empty set
	{
		auto result_xb = zdd.union_(zdd_x, zdd.bottom());
		auto result_bx = zdd.union_(zdd.bottom(), zdd_x);
		CHECK(result_xb == result_bx);
		CHECK(result_xb == zdd_x);
	}
}

TEST_CASE("ZDD tautology", "[zdd]")
{
	using namespace bill;
	std::ostringstream os;
	zdd_base zdd(3);

	SECTION("Tautology(0) == Universe == Power set")
	{
		auto result = zdd.tautology();
		zdd.print_sets(result, os);
		CHECK(os.str() == "{  }\n{ 2 }\n{ 1 }\n{ 1, 2 }\n{ 0 }\n{ 0, 2 }\n{ 0, 1 }\n"
		                  "{ 0, 1, 2 }\n");
	}
}
