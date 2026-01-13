#pragma once

#include <vector>
#include <span>

namespace Dithering
{

  std::vector<std::byte> Apply(std::span<std::byte> image,
      int imageWidth, int imageHeight,
      std::span<uint32_t> palette,
      int mode);

} //Dithering
