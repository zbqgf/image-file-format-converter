#include "Palette.h"
#include "Helpers.h"

#include <algorithm>
#include <functional>

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

  std::vector<uint32_t> GenerateMedianCut(
      std::span<std::byte> image, int imageWidth, int imageHeight)
  {
    size_t pixelCount = imageWidth * imageHeight;
    uint32_t* imageData = reinterpret_cast<uint32_t*>(image.data());
    std::vector<uint32_t> imageDataCopy(imageData, imageData + pixelCount);

    std::vector<uint32_t> result;
    result.reserve(kColorCount);

    std::function<void(int, int, int)> medianCut;
    medianCut = [&](int start, int end, int depth) {
      if (depth == 0 || end - start <= 1) {
        uint64_t r = 0, g = 0, b = 0;
        int count = end - start;

        for (int i = start; i < end; ++i) {
          auto pixelColorUnpacked = Helpers::UnpackColor(imageDataCopy[i]);
          r += pixelColorUnpacked[0];
          g += pixelColorUnpacked[1];
          b += pixelColorUnpacked[2];
        }

        result.push_back(Helpers::PackColor(
              static_cast<uint8_t>(r / count),
              static_cast<uint8_t>(g / count),
              static_cast<uint8_t>(b / count),
              255));

        return;
      }

      uint8_t rMin = 255, rMax = 0;
      uint8_t gMin = 255, gMax = 0;
      uint8_t bMin = 255, bMax = 0;

      for (int i = start; i < end; ++i) {
        auto pixelColorUnpacked = Helpers::UnpackColor(imageDataCopy[i]);

        rMin = std::min(rMin, pixelColorUnpacked[0]);
        rMax = std::max(rMax, pixelColorUnpacked[0]);

        gMin = std::min(gMin, pixelColorUnpacked[1]);
        gMax = std::max(gMax, pixelColorUnpacked[1]);

        bMin = std::min(bMin, pixelColorUnpacked[2]);
        bMax = std::max(bMax, pixelColorUnpacked[2]);
      }

      int rRange = rMax - rMin;
      int gRange = gMax - gMin;
      int bRange = bMax - bMin;

      if (rRange >= gRange && rRange >= bRange) {
        std::sort(imageDataCopy.begin() + start, imageDataCopy.begin() + end,
            [](uint32_t a, uint32_t b) {
            return Helpers::UnpackColor(a)[0] <
            Helpers::UnpackColor(b)[0];
            });
      } else if (gRange >= bRange) {
        std::sort(imageDataCopy.begin()  + start, imageDataCopy.begin() + end,
            [](uint32_t a, uint32_t b) {
            return Helpers::UnpackColor(a)[1] <
            Helpers::UnpackColor(b)[1];
            });
      } else {
        std::sort(imageDataCopy.begin() + start, imageDataCopy.begin() + end,
            [](uint32_t a, uint32_t b) {
            return Helpers::UnpackColor(a)[2] <
            Helpers::UnpackColor(b)[2];
            });
      }

      int mid = (start + end) / 2;

      medianCut(start, mid, depth - 1);
      medianCut(mid, end, depth - 1);
    };

    int depth = 0;
    for (int n = kColorCount; n > 1; n >>= 1) {
      ++depth;
    }

    medianCut(0, pixelCount, depth);

    return result;
  } 

  std::vector<uint32_t> GenerateMedianCutMono(
      std::span<std::byte> image, int imageWidth, int imageHeight)
  {
    size_t pixelCount = imageWidth * imageHeight;
    uint32_t* imageData = reinterpret_cast<uint32_t*>(image.data());
    std::vector<uint32_t> imageDataCopy(imageData, imageData + pixelCount);

    std::vector<uint32_t> result;
    result.reserve(kColorCount);

    auto Luminance = [](uint32_t color) {
      auto colorUnpacked = Helpers::UnpackColor(color);

      return 0.299f * colorUnpacked[0]
        + 0.587f * colorUnpacked[1]
        + 0.114f * colorUnpacked[2];
    };

    std::function<void(int, int, int)> MedianCut;
    MedianCut = [&](int start, int end, int depth) {
      if (depth == 0 || end - start <= 1) {
        float sum = 0;
        int count = end - start;

        for (int i = start; i < end; ++i) {
          sum += Luminance(imageDataCopy[i]);
        }

        uint8_t l = static_cast<uint8_t>(sum / count);
        result.push_back(Helpers::PackColor(l, l, l, 255));

        return;
      }

      std::sort(imageDataCopy.begin() + start, imageDataCopy.begin() + end,
          [&](uint32_t a, uint32_t b) {
          return Luminance(a) < Luminance(b);
          });

      int mid = (start + end) / 2;

      MedianCut(start, mid, depth - 1);
      MedianCut(mid, end, depth - 1);
    };

    int depth = 0;
    for (int n = kColorCount; n > 1; n >>= 1) {
      ++depth;
    }

    MedianCut(0, pixelCount, depth);

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
    std::span<std::byte> image, int imageWidth, int imageHeight, int mode)
{
  if (mode == 0) return GeneratePosterized();
  else if (mode == 1) return GeneratePosterizedMono();
  else if (mode == 2) return GenerateMedianCut(image, imageWidth, imageHeight);
  else if (mode == 3) return GenerateMedianCutMono(image, imageWidth, imageHeight);
}

