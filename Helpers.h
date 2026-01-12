#pragma once

#include <array>

namespace Helpers
{

  inline std::array<uint8_t, 4> UnpackColor(uint32_t color)
  {
    return {
      (color >>  0) & 0xff,
      (color >>  8) & 0xff,
      (color >> 16) & 0xff,
      (color >> 24) & 0xff
    };
  }

  inline uint32_t PackColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
  {
    uint32_t result = static_cast<uint32_t>(r);
    result |= static_cast<uint32_t>(g) << 8;
    result |= static_cast<uint32_t>(b) << 16;
    result |= static_cast<uint32_t>(a) << 24;

    return result;
  }

} //Helpers
