#pragma once
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace fileManagement {
    struct DG5ImageData {
        std::vector<std::byte> image = {};
        int width = 0;
        int height = 0;
    };

    void saveToFile(std::vector<std::byte>& image, std::filesystem::path path, int width, int height, int mode, int dithering);
    DG5ImageData loadFromFile(std::filesystem::path path);
}
