#include "Palette.h"
#include "Helpers.h"

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
      result[i] = Helpers::PackColor(r, g, b, 255);
    }

    return result;
  }

  std::vector<uint32_t> GeneratePosterizedMono()
  {
    std::vector<uint32_t> result(kColorCount);

    for (int i = 0; i < kColorCount; ++i) {
      uint8_t l = i / 31.0f * 255.0f;
      result[i] = Helpers::PackColor(l, l, l, 255);
    }

    return result;
  }

}

uint32_t Palette::FindClosestColorFromPalette(
    uint32_t color, std::span<const uint32_t> palette)
{
  auto ColorDistanceSq = [](uint32_t a, uint32_t b) {
    auto unpackedA = Helpers::UnpackColor(a);
    auto unpackedB = Helpers::UnpackColor(b);

    int dr = static_cast<int>(unpackedA[0]) - static_cast<int>(unpackedB[0]);
    int db = static_cast<int>(unpackedA[1]) - static_cast<int>(unpackedB[1]);
    int dg = static_cast<int>(unpackedA[2]) - static_cast<int>(unpackedB[2]);

    return dr*dr + dg*dg + db*db;
  };

  uint32_t closestColor = palette[0];
  int closestColorDist = ColorDistanceSq(color, closestColor);
  for (uint32_t paletteColor : palette) {
    int colorDist = ColorDistanceSq(color, paletteColor);
    if (colorDist < closestColorDist) {
      closestColor = paletteColor;
      closestColorDist = colorDist; 
    }
  };

  return closestColor;
}

std::vector<uint32_t> Palette::Generate(
    std::span<std::byte> image, int mode)
{
  if (mode == 0) return GeneratePosterized();
  return GeneratePosterizedMono();
}

