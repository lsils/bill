/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "../catch2.hpp"
#include "cudd_wrapper.hpp"

TEST_CASE("CUDD ZDD choose", "[cudd]")
{
  cudd::cudd_zdd zdd( 5 );

  auto every = zdd.bottom();

  every = zdd.union_( every, zdd.elementary( 0 ) );
  every = zdd.union_( every, zdd.elementary( 1 ) );
  every = zdd.union_( every, zdd.elementary( 2 ) );
  zdd.print( every, "every" );

  auto const& n = zdd.choose( every, 2 );
  zdd.print( n, "choose" );
}

TEST_CASE("CUDD ZDD ops", "[cudd]")
{
  cudd::cudd_zdd zdd( 5 );
  
  auto every = zdd.bottom();
  every = zdd.union_( every, zdd.elementary( 0 ) );
  every = zdd.union_( every, zdd.elementary( 1 ) );
  every = zdd.union_( every, zdd.elementary( 3 ) );
  zdd.print( every, "every" );

  std::vector<uint32_t> vec;
  vec.push_back( 0 ); vec.push_back( 1 ); vec.push_back( 3 );
  auto const& rest = zdd.dont_care( vec );
  zdd.print( rest, "rest" );

  auto x = zdd.bottom();
  x = zdd.union_( x, zdd.choose( every, 1 ) );
  x = zdd.union_( x, zdd.choose( every, 3 ) );
  zdd.print( x, "choose" );

  auto const& res = zdd.join( x, rest );
  zdd.print( res, "final" );
}

