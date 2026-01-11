#pragma once

#include <span>
#include <vector>

namespace Quantization
{

std::vector<std::byte> apply(std::span<std::byte> image,
    int imageWidth, int imageHeight,
    std::span<uint32_t> palette);

} //Quantization
