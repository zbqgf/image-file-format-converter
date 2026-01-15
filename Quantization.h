#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace Quantization
{

  std::vector<std::byte> Apply(std::span<std::byte> image,
      int imageWidth, int imageHeight,
      std::span<std::uint32_t> palette);

} //Quantization
