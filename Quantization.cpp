#include "Quantization.h"
#include "Palette.h"

#include <array>

std::vector<std::byte>
Quantization::Apply(std::span<std::byte> image,
    int imageWidth, int imageHeight,
    std::span<uint32_t> palette)
{
  size_t resultSize = imageWidth * imageHeight * 4;
  std::vector<std::byte> result(resultSize);

  uint32_t* imageData = reinterpret_cast<uint32_t*>(image.data());
  uint32_t* resultData = reinterpret_cast<uint32_t*>(result.data());

  for (int i = 0; i < imageHeight; ++i) {
    for (int j = 0; j < imageWidth; ++j) {
      int idx = j + i * imageWidth;

      uint32_t closestColor = Palette::FindClosestColorFromPalette(
          imageData[idx], palette);

      resultData[idx] = closestColor;
    }
  }

  return result;
}
