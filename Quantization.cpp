#include "Quantization.h"

namespace
{

  uint8_t GetR(uint32_t c) { return  c        & 0xFF; }
  uint8_t GetG(uint32_t c) { return (c >>  8) & 0xFF; }
  uint8_t GetB(uint32_t c) { return (c >> 16) & 0xFF; }
  uint8_t GetA(uint32_t c) { return (c >> 24) & 0xFF; }

  int ColorDistanceSq(uint32_t a, uint32_t b)
  {
    int dr = int(GetR(a)) - int(GetR(b));
    int dg = int(GetG(a)) - int(GetG(b));
    int db = int(GetB(a)) - int(GetB(b));
    return dr*dr + dg*dg + db*db;
  }

}

std::vector<std::byte>
Quantization::apply(std::span<std::byte> image,
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

      uint32_t bestColor = palette[0];
      for (uint32_t paletteColor : palette) {
        if (ColorDistanceSq(imageData[idx], paletteColor) <
            ColorDistanceSq(imageData[idx], bestColor)) {
          bestColor = paletteColor;
        }
      }
      resultData[idx] = (bestColor & 0x00FFFFFF)
        | (imageData[idx] & 0xFF000000);
    }
  }

  return result;
}
