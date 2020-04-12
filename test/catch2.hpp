/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/

#include <bill/utils/platforms.hpp>

#if defined(BILL_WINDOWS_PLATFORM)
#pragma warning(push)
#pragma warning(disable : 4365 4514 4571 4583 4619 4623 4625 4626 4710 4711 4774 4820 4820 4868 5026 5027 5039)
#include <catch.hpp>
#pragma warning(pop)
#else
#include <catch.hpp>
#endif
