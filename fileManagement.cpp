#include "fileManagement.h"

#include <cmath>
#include <fstream>

#include "Palette.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_stdinc.h"

Uint8 convertRGBAto5b(std::byte &red, std::byte &green, std::byte &blue) {
    Uint8 R = (static_cast<Uint8>(red) * 3 + 127) / 255;
    Uint8 G = (static_cast<Uint8>(green) * 3 + 127) / 255;
    Uint8 B = (static_cast<Uint8>(blue) + 127) / 255;

    return (R << 3) | (G << 1) | B;
}

SDL_Color convert5btoRGBA(Uint8& color5b) {
    SDL_Color color;

    color.r = static_cast<Uint8>(round(((color5b>>3)&3)*255.0/3.0));
    color.g = static_cast<Uint8>(round(((color5b>>1)&3)*255.0/3.0));
    color.b = static_cast<Uint8>(round(((color5b>>0)&1)*255.0/1.0));

    return color;
}

void fileManagement::saveToFile(std::vector<std::byte>& image, std::filesystem::path path, int width, int height, int mode, int dithering) {
    std::ofstream file(path, std::ios::binary);
    const int PIXELS_PER_BLOCK = 8;
    const int BYTES_PER_BLOCK = 5;
    int columns = (width + PIXELS_PER_BLOCK - 1) / PIXELS_PER_BLOCK;


    if (!file) {
        throw std::invalid_argument("Can't open file");
    }

    file.write("DG", 2);
    file.write(reinterpret_cast<const char*>(&width), 2);
    file.write(reinterpret_cast<const char*>(&height), 2);
    file.write(reinterpret_cast<const char*>(&mode), 1);
    file.write(reinterpret_cast<const char*>(&dithering), 1);
    int temp = columns * height * BYTES_PER_BLOCK;
    file.write(reinterpret_cast<const char*>(&temp), 4);

    for (auto color: Palette::Generate(image, width, height, mode)) {
        file.put(static_cast<const char>((color >> 16) & 0xFF));
        file.put(static_cast<const char>((color >> 8) & 0xFF));
        file.put(static_cast<const char>(color & 0xF));
    }

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

fileManagement::DG5ImageData fileManagement::loadFromFile(std::filesystem::path path) {
    std::ifstream file(path, std::ios::binary);
    DG5ImageData image;
    int imageSize;
    const int PIXELS_PER_BLOCK = 8;
    const int BYTES_PER_BLOCK = 5;

    if (!file) {
        throw std::invalid_argument("Can't open file");
    }

    Uint16 szerokoscObrazka = 0;
    Uint16 wysokoscObrazka = 0;
    Uint8 ileBitow = 0;
    char identyfikator[3] = {0};

    file.seekg(2, std::ios::beg);
    file.read(reinterpret_cast<char*>(&image.width), 2);
    file.read(reinterpret_cast<char*>(&image.height), 2);
    file.seekg(102, std::ios::cur);

    int columns = (image.width + PIXELS_PER_BLOCK - 1) / PIXELS_PER_BLOCK;
    image.image.resize(image.width * image.height * 4);

    for (int col = 0; col < columns; col++) {
        int startX = col * PIXELS_PER_BLOCK;

        for (int y = 0; y < image.height; y++) {
            Uint8 inputBytes[5];
            file.read(reinterpret_cast<char*>(&inputBytes), sizeof(Uint8) * BYTES_PER_BLOCK);

            if (file.eof()) break;

            Uint8 pixels[8] = {0};

            for (int bitPos = 0; bitPos < 5; bitPos++) {
                Uint8 byteValue = inputBytes[bitPos];
                for (int pixelIdx = 0; pixelIdx < 8; pixelIdx++) {
                    if (byteValue & (1 << pixelIdx)) {
                        pixels[pixelIdx] |= (1 << bitPos);
                    }
                }
            }

            for (int i = 0; i < PIXELS_PER_BLOCK; i++) {
                int x = startX + i;
                if (x < image.width) {
                    SDL_Color color = convert5btoRGBA(pixels[i]);
                    int pixelIndex = (y * image.width + x) * 4;

                    image.image[pixelIndex] = static_cast<std::byte>(color.r);
                    image.image[pixelIndex + 1] = static_cast<std::byte>(color.g);
                    image.image[pixelIndex + 2] = static_cast<std::byte>(color.b);
                }
            }
        }
    }

    file.close();

    return image;
};
