/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include <bill/utils/platforms.hpp>

#if defined(BILL_WINDOWS_PLATFORM)
#pragma warning(push, 0)
#include <catch.hpp>
#pragma warning(pop)
#else
#include <catch.hpp>
#endif
