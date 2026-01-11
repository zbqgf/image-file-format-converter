#include <SDL3/SDL.h>

#include <iostream>
#include <ostream>
#include <fstream>
#include <filesystem>
#include <array>
#include <span>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_BMP
#include "stb_image.h"

#include "MyImGui.h"
#include "PaletteGenerator.h"
#include "Quantization.h"

static SDL_Window *gWindow;
static SDL_Renderer *gRenderer;

static int gWidth = 1280;
static int gHeight = 720;

static int gCurrentMode;
static int gCurrentDithering;
static bool gEnablePreview;
static std::vector<uint32_t> gPalette;

static std::vector<std::byte> LoadFromFile(
    const std::filesystem::path& path, int& x, int& y)
{
  auto bufferSize = std::filesystem::file_size(path);
  std::vector<std::byte> buffer(bufferSize);
  std::ifstream(path, std::ios::binary).read(
      reinterpret_cast<char*>(buffer.data()), bufferSize);

  unsigned char* imageData = stbi_load_from_memory(
      reinterpret_cast<const unsigned char*>(buffer.data()),
      static_cast<int>(bufferSize),
      &x, &y, 0, 4);

  const size_t byteCount = x * y * 4;
  std::vector<std::byte> result(byteCount);
  std::memcpy(result.data(), imageData, byteCount);

  stbi_image_free(imageData);

  return result;
}

static std::vector<std::byte> ProcessImage(
    std::span<std::byte> originalImage,
    int imageWidth, int imageHeight, int mode/*, int dithering*/)
{
  gPalette = PaletteGenerator::Generate(originalImage, mode);

  //if (dithering == 0) {
    return Quantization::apply(originalImage,
        imageWidth, imageHeight, gPalette);
  //}

  //return Dithering::apply(originalImage,
  //    imageWidth, imageHeight, gPalette);
}

int main(int /*argc*/, char **/*argv*/)
{
  SDL_Init(SDL_INIT_VIDEO);

  float mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

  SDL_CreateWindowAndRenderer("Projekt", gWidth, gHeight, 0, &gWindow, &gRenderer);

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.Fonts->AddFontFromFileTTF("assets/Inter-Regular.ttf");
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  style.ScaleAllSizes(mainScale);
  style.FontScaleDpi = mainScale;

  ImGui_ImplSDL3_InitForSDLRenderer(gWindow, gRenderer);
  ImGui_ImplSDLRenderer3_Init(gRenderer);

  int imageWidth, imageHeight;
  std::vector<std::byte> originalImage = LoadFromFile(
      "assets/obrazek1.bmp", imageWidth, imageHeight);

  std::vector<std::byte> processedImage = ProcessImage(
      originalImage, imageWidth, imageHeight, gCurrentMode);

  SDL_Texture* texture = SDL_CreateTexture(
      gRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC,
      imageWidth, imageHeight);

  SDL_UpdateTexture(texture, 0, processedImage.data(), imageWidth * 4); 

  bool shouldQuit = false;
  while (!shouldQuit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) {
        shouldQuit = true;
        break;
      }
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::DockSpaceOverViewport();

    ImGui::Begin("Ustawienia");

    if (ImGui::Combo("Tryb", &gCurrentMode,
          "Paleta Kolorowa Narzucona\0"
          "Odcienie Szarości Narzucone\0"
          "Paleta Kolorowa Dedykowana\0"
          "Odcienie Szarości Dedykowane\0"))
    {
      processedImage = ProcessImage(
          originalImage, imageWidth, imageHeight, gCurrentMode);
      SDL_UpdateTexture(
          texture, nullptr, processedImage.data(), imageWidth * 4); 
    }

    if (ImGui::Combo("Dithering", &gCurrentDithering,
          "Brak\0Bayer\0Floyd-Steinberg\0"))
    {
      processedImage = ProcessImage(
          originalImage, imageWidth, imageHeight, gCurrentMode);
      SDL_UpdateTexture(
          texture, nullptr, processedImage.data(), imageWidth * 4); 
    }

    MyImGui::SettingsPalette(gPalette);

    ImGui::End();

    ImGui::Begin("obrazek1.bmp");

    ImVec2 displaySize = ImVec2(imageWidth, imageHeight);
    MyImGui::Image((ImTextureID)texture, displaySize);

    ImGui::End();

    ImGui::Render();
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xff);
    SDL_RenderClear(gRenderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), gRenderer);
    SDL_RenderPresent(gRenderer);
  }

  SDL_Quit();
}
