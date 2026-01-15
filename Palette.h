#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace Palette
{
  std::uint32_t FindClosestColorFromPalette(std::uint32_t color, std::span<const std::uint32_t> palette);

  std::vector<std::uint32_t> Generate(std::span<std::byte> image, int imageWidth, int imageHeight, int mode);

} //Palette

