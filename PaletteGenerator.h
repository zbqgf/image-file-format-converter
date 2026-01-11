#pragma once

#include <span>
#include <vector>

namespace PaletteGenerator
{

  std::vector<uint32_t> Generate(std::span<std::byte> image, int mode);

} //PaletteGenerator

