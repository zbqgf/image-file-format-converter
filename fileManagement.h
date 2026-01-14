#pragma once
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace fileManagement {
    void saveToFile(std::vector<std::byte>& image, std::filesystem::path path, int width, int height, int mode, int dithering);
    std::vector<std::byte> loadFromFile(std::filesystem::path path);
}
