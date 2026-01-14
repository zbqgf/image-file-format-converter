#include "fileManagement.h"

#include <cmath>
#include <fstream>

#include "Palette.h"
#include "SDL3/SDL_stdinc.h"

Uint8 convertRGBAto5b(std::byte& red, std::byte& green, std::byte& blue) {
    return ((static_cast<int>(red) / 64) << 3) | ((static_cast<int>(green) / 64) << 1) | (static_cast<int>(blue) / 128);
}

void fileManagement::saveToFile(std::vector<std::byte>& image, std::filesystem::path path, int width, int height, int mode, int dithering) {
    std::ofstream file(path, std::ios::binary);

    if (!file) {
        throw std::invalid_argument("Can't open file");
    }

    file.write("DG", 2);
    file.write(reinterpret_cast<const char*>(&width), 2);
    file.write(reinterpret_cast<const char*>(&width), 2);
    file.write(reinterpret_cast<const char*>(&mode), 1);
    file.write(reinterpret_cast<const char*>(&dithering), 1);
    file.write(reinterpret_cast<const char*>("96"), 4);

    for (auto color: Palette::Generate(image, width, height, mode)) {
        file.put(static_cast<const char>((color >> 16) & 0xFF));
        file.put(static_cast<const char>((color >> 8) & 0xFF));
        file.put(static_cast<const char>(color & 0xF));
    }

    const int PIXELS_PER_BLOCK = 8;
    const int BYTES_PER_BLOCK = 5;

    int columns = (width + PIXELS_PER_BLOCK - 1) / PIXELS_PER_BLOCK;

    for (int col = 0; col < columns; col++) {
        int startX = col * PIXELS_PER_BLOCK;

        for (int y = 0; y < height; y++) {
            Uint8 pixels[8] = {0};

            for (int i = 0; i < PIXELS_PER_BLOCK; i++) {
                int x = startX + i;
                if (x < width) {
                    int pixelsLocation = (y * width + x) * 4;

                    pixels[i] = convertRGBAto5b(
                        image[pixelsLocation],
                        image[pixelsLocation + 1],
                        image[pixelsLocation + 2]
                    );
                }
            }

            Uint8 outputBytes[5] = {0};

            for (int bitPos = 0; bitPos < 5; bitPos++) {
                Uint8 byteValue = 0;
                for (int pixelIdx = 0; pixelIdx < 8; pixelIdx++) {
                    if (pixels[pixelIdx] & (1 << bitPos)) {
                        byteValue |= (1 << pixelIdx);
                    }
                }
                outputBytes[bitPos] = byteValue;
            }

            file.write(reinterpret_cast<const char*>(outputBytes), sizeof(Uint8) * BYTES_PER_BLOCK);
        }
    }

    file.close();
};

std::vector<std::byte> fileManagement::loadFromFile(std::filesystem::path path) {
    return std::vector<std::byte>();
};
