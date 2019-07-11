/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include <bill/utils/platforms.hpp>

#if defined(BILL_WINDOWS_PLATFORM)
#pragma warning(push)
#pragma warning(disable:4365)
#pragma warning(disable:4514)
#pragma warning(disable:4571)
#pragma warning(disable:4583)
#pragma warning(disable:4619)
#pragma warning(disable:4623)
#pragma warning(disable:4625)
#pragma warning(disable:4626)
#pragma warning(disable:4710)
#pragma warning(disable:4711)
#pragma warning(disable:4774)
#pragma warning(disable:4820)
#pragma warning(disable:4820)
#pragma warning(disable:5026)
#pragma warning(disable:5027)
#pragma warning(disable:5039)
#include <catch.hpp>
#pragma warning(pop)
#else
#include <catch.hpp>
#endif
