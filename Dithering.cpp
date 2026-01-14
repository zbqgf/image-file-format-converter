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
    size_t pixelCount = imageWidth * imageHeight;
    size_t resultSize = pixelCount * 4;
    std::vector<std::byte> result(resultSize);

    uint32_t* imageData = reinterpret_cast<uint32_t*>(image.data());
    uint32_t* resultData = reinterpret_cast<uint32_t*>(result.data());

    std::vector<float> rErrors(pixelCount, 0.0f);
    std::vector<float> gErrors(pixelCount, 0.0f);
    std::vector<float> bErrors(pixelCount, 0.0f);

    for (int i = 0; i < imageHeight; ++i) {
      for (int j = 0; j < imageWidth; ++j) {
        int idx = j + i * imageWidth;

        auto unpackedPixelColor = Helpers::UnpackColor(imageData[idx]);

        uint8_t r = ClampToByte(unpackedPixelColor[0] + rErrors[idx]);
        uint8_t g = ClampToByte(unpackedPixelColor[1] + gErrors[idx]);
        uint8_t b = ClampToByte(unpackedPixelColor[2] + bErrors[idx]);

        uint32_t closestColor = Palette::FindClosestColorFromPalette(
            Helpers::PackColor(r, g, b, unpackedPixelColor[3]), palette);

        resultData[idx] = closestColor;

        auto closestColorUnpacked = Helpers::UnpackColor(closestColor);

        int rError = r - closestColorUnpacked[0];
        int gError = g - closestColorUnpacked[1];
        int bError = b - closestColorUnpacked[2];

        if (j + 1 < imageWidth) {
          size_t s = j + 1 + i * imageWidth;
          rErrors[s] += rError * 7 / 16.0f;
          gErrors[s] += gError * 7 / 16.0f;
          bErrors[s] += bError * 7 / 16.0f;
        }

        if (j > 0 && i + 1 < imageHeight) {
          size_t s = j - 1 + (i + 1) * imageWidth;
          rErrors[s] += rError * 3 / 16.0f;
          gErrors[s] += gError * 3 / 16.0f;
          bErrors[s] += bError * 3 / 16.0f;
        }

        if (i + 1 < imageHeight) {
          size_t s = j + (i + 1) * imageWidth;
          rErrors[s] += rError * 5 / 16.0f;
          gErrors[s] += gError * 5 / 16.0f;
          bErrors[s] += bError * 5 / 16.0f;
        }

        if (j + 1 < imageWidth && i + 1 < imageHeight) {
          size_t s = j + 1 + (i + 1) * imageWidth;
          rErrors[s] += rError * 1 / 16.0f;
          gErrors[s] += gError * 1 / 16.0f;
          bErrors[s] += bError * 1 / 16.0f;
        }
      }
    }

    return result;
  }

}

std::vector<std::byte>
Dithering::Apply(std::span<std::byte> image,
    int imageWidth, int imageHeight,
    std::span<uint32_t> palette,
    int mode)
{
  if (mode == 1) {
    return ApplyBayerDithering(image, imageWidth, imageHeight, palette);
  } else if (mode == 2) {
    return ApplyFloydSteinbergDithering(image, imageWidth, imageHeight, palette);
  }
}
