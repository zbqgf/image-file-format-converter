#pragma once

#include <span>
#include <vector>

namespace Palette
{

  uint32_t FindClosestColorFromPalette(uint32_t color, std::span<const uint32_t> palette);

  std::vector<uint32_t> Generate(std::span<std::byte> image, int imageWidth, int imageHeight, int mode);

} //Palette

