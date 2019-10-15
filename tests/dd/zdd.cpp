/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "../catch2.hpp"

#include <bill/dd/zdd.hpp>
#include <sstream>

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
		CHECK(zdd.num_nodes() == 1u);
		CHECK(zdd_0 == 2u);
		CHECK(zdd.count_nodes(zdd_0) == 1u);
		CHECK(zdd.count_sets(zdd_0) == 1u);
	}
	SECTION("An ZDD base with 4096 variable")
	{
		zdd_base zdd(4095);
		CHECK(zdd.num_nodes() == 4095u);
		CHECK(zdd.elementary(4094u) == 4096);
	}
}

TEST_CASE("ZDD Operations", "[zdd]")
{
	using namespace bill;
	std::ostringstream os;
	zdd_base zdd(2);
	auto zdd_0 = zdd.elementary(0);
	auto zdd_1 = zdd.elementary(1);
	SECTION("Difference (empty)")
	{
		auto zdd_diff = zdd.difference(zdd_0, zdd_0);
		CHECK(zdd.count_nodes(zdd_diff) == 0u);
		CHECK(zdd.count_sets(zdd_diff) == 0u);
		zdd.print_sets(zdd_diff, os);
		CHECK(os.str() == "");
	}
	SECTION("Difference (nonempty)")
	{
		auto zdd_diff = zdd.difference(zdd_0, zdd_1);
		CHECK(zdd.count_nodes(zdd_diff) == 1u);
		CHECK(zdd.count_sets(zdd_diff) == 1u);
		zdd.print_sets(zdd_diff, os);
		CHECK(os.str() == "{ 0 }\n");

		os.str("");
		zdd_diff = zdd.difference(zdd_1, zdd_0);
		CHECK(zdd.count_nodes(zdd_diff) == 1u);
		CHECK(zdd.count_sets(zdd_diff) == 1u);
		zdd.print_sets(zdd_diff, os);
		CHECK(os.str() == "{ 1 }\n");
	}
	SECTION("Intesection (empty)")
	{
		auto zdd_intersec = zdd.intersection(zdd_0, zdd_1);
		CHECK(zdd.count_nodes(zdd_intersec) == 0u);
		CHECK(zdd.count_sets(zdd_intersec) == 0u);
		zdd.print_sets(zdd_intersec, os);
		CHECK(os.str() == "");

		auto zdd_join = zdd.join(zdd_0, zdd_1);
		zdd_intersec = zdd.intersection(zdd_0, zdd_join);
		CHECK(zdd.count_nodes(zdd_intersec) == 0u);
		CHECK(zdd.count_sets(zdd_intersec) == 0u);
		zdd.print_sets(zdd_intersec, os);
		CHECK(os.str() == "");
	}
	SECTION("Intesection (nonempty)")
	{
		auto zdd_join = zdd.union_(zdd_0, zdd_1);
		auto zdd_intersec = zdd.intersection(zdd_0, zdd_join);
		CHECK(zdd.count_nodes(zdd_intersec) == 1u);
		CHECK(zdd.count_sets(zdd_intersec) == 1u);
		zdd.print_sets(zdd_intersec, os);
		CHECK(os.str() == "{ 0 }\n");
	}
	SECTION("Join")
	{
		auto zdd_join = zdd.join(zdd_0, zdd_1);
		CHECK(zdd.count_nodes(zdd_join) == 2u);
		CHECK(zdd.count_sets(zdd_join) == 1u);
		zdd.print_sets(zdd_join, os);
		CHECK(os.str() == "{ 0, 1 }\n");
	}
	SECTION("Union with TOP")
	{
		auto zdd_union = zdd.union_(zdd_0, zdd.top());
		CHECK(zdd.count_nodes(zdd_union) == 1u);
		CHECK(zdd.count_sets(zdd_union) == 2u);
		zdd.print_sets(zdd_union, os);
		CHECK(os.str() == "{  }\n{ 0 }\n");
	}
	SECTION("Union between two ZDD")
	{
		auto zdd_union = zdd.union_(zdd_0, zdd_1);
		CHECK(zdd.count_nodes(zdd_union) == 2u);
		CHECK(zdd.count_sets(zdd_union) == 2u);
		zdd.print_sets(zdd_union, os);
		CHECK(os.str() == "{ 1 }\n{ 0 }\n");
	}
}
