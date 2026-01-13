#include "Dithering.h"
#include "Palette.h"
#include "Helpers.h"

namespace
{

  uint8_t ClampToByte(int v)
  {
    return (v < 0) ? 0 : (v > 255 ? 255 : v);
  }

  constexpr float kBayer[] = {
    6.0f/16.0f, 14.0f/16.0f, 8.0f/16.0f, 16.0f/16.0f,
    10.0f/16.0f, 2.0f/16.0f, 12.0f/16.0f, 4.0f/16.0f,
    7.0f/16.0f, 15.0f/16.0f, 5.0f/16.0f, 13.0f/16.0f,
    11.0f/16.0f, 3.0f/16.0f, 9.0f/16.0f, 1.0f/16.0f
  };


  std::vector<std::byte> ApplyBayerDithering(
      std::span<std::byte> image,
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

        int x = j % 4, y = i % 4;
        float threshold = kBayer[x + y * 4] - 0.5f;

        auto unpackedPixelColor = Helpers::UnpackColor(imageData[idx]);

        uint8_t r = ClampToByte(static_cast<int>(unpackedPixelColor[0])
            + static_cast<int>(threshold * 31.0f));
        uint8_t g = ClampToByte(static_cast<int>(unpackedPixelColor[1])
            + static_cast<int>(threshold * 31.0f));
        uint8_t b = ClampToByte(static_cast<int>(unpackedPixelColor[2])
            + static_cast<int>(threshold * 31.0f));

        uint32_t closestColor = Palette::FindClosestColorFromPalette(
            Helpers::PackColor(r, g, b, unpackedPixelColor[3]), palette);

        resultData[idx] = closestColor;
      }
    }

    return result;
  }

  std::vector<std::byte> ApplyFloydSteinbergDithering(
      std::span<std::byte> image,
      int imageWidth, int imageHeight,
      std::span<uint32_t> palette)
  {
    return {};
  }

}

std::vector<std::byte>
Dithering::Apply(std::span<std::byte> image,
    int imageWidth, int imageHeight,
    std::span<uint32_t> palette,
    int mode)
{
  if (mode == 1) return ApplyBayerDithering(image, imageWidth, imageHeight, palette);
  else if (mode == 2) return ApplyFloydSteinbergDithering(image, imageWidth, imageHeight, palette);
}
