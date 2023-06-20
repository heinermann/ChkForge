#pragma once

#include <cstdint>

using u16 = std::uint16_t;
using u32 = std::uint32_t;

namespace Chk {

  constexpr size_t TotalUnitTypes = 228;
  constexpr size_t TotalForces = 4;


  constexpr size_t TotalOriginalLocations = 64;
  constexpr size_t TotalLocations = 255;

  __declspec(align(1)) struct Location
  {
    u32 left;
    u32 top;
    u32 right;
    u32 bottom;
    u16 stringId;
    u16 elevationFlags;
  };

}
