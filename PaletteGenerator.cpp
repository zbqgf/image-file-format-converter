#include "PaletteGenerator.h"

namespace
{
  constexpr int kColorCount = 32;

  std::vector<uint32_t> GeneratePosterized()
  {
    std::vector<uint32_t> result(kColorCount);

    for (int i = 0; i < kColorCount; ++i) {
      uint8_t r = ((i >> 3) & 3) / 3.0f * 255.0f;
      uint8_t g = ((i >> 1) & 3) / 3.0f * 255.0f;
      uint8_t b = ((i >> 0) & 1) / 1.0f * 255.0f;
      result[i] = (r << 0) | (g << 8) | (b << 16) | (255 << 24);
    }

    return result;
  }

  std::vector<uint32_t> GeneratePosterizedMono()
  {
    std::vector<uint32_t> result(kColorCount);

    for (int i = 0; i < kColorCount; ++i) {
      uint8_t l = i / 31.0f * 255.0f;
      result[i] = (l << 0) | (l << 8) | (l << 16) | (255 << 24);
    }

    return result;
  }
}

std::vector<uint32_t> PaletteGenerator::Generate(
    std::span<std::byte> image, int mode)
{
  if (mode == 0) return GeneratePosterized();
  return GeneratePosterizedMono();
}

