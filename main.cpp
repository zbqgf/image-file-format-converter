#include <SDL3/SDL.h>

#include <iostream>
#include <ostream>
#include <fstream>
#include <filesystem>
#include <array>
#include <span>
#include <vector>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_ONLY_BMP
#include <cstring>

#include "stb_image.h"
#include "stb_image_write.h"

#include "MyImGui.h"
#include "Palette.h"
#include "Quantization.h"
#include "Dithering.h"
#include "fileManagement.h"

struct AppState
{
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;

  int width = 1280;
  int height = 720;

  int mode = 0;
  int dithering = 0;
  bool enablePreview = 0;

  std::vector<std::byte> originalImage;
  std::vector<std::byte> processedImage;
  int imageWidth, imageHeight;

  SDL_Texture* texture = nullptr;

  std::vector<uint32_t> palette;

  bool hasPendingOpen = false;
  std::filesystem::path pendingOpenPath;

  bool hasPendingSave = false;
  std::filesystem::path pendingSavePath;
};

static AppState gApp;

static std::vector<std::byte> LoadBMP(
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

static void WriteBMP(
    std::filesystem::path& path, std::span<std::byte> image, int x, int y)
{
  void* data = reinterpret_cast<void*>(image.data());
  stbi_write_bmp(path.string().c_str(), x, y, 4, data);
}

static std::vector<std::byte> ProcessImage(
    std::span<std::byte> originalImage,
    int imageWidth, int imageHeight, int mode, int dithering)
{
  gApp.palette = Palette::Generate(
      originalImage, imageWidth, imageHeight, mode);

  if (dithering == 0) {
    return Quantization::Apply(
        originalImage, imageWidth, imageHeight, gApp.palette);
  }

  return Dithering::Apply(
        originalImage, imageWidth, imageHeight, gApp.palette, dithering);
}

void ReprocessImage(AppState& app)
{
  if (!app.originalImage.empty()) {
    app.processedImage = ProcessImage(
        gApp.originalImage,
        gApp.imageWidth,
        gApp.imageHeight,
        gApp.mode,
        gApp.dithering);

    if (!app.texture) {
      app.texture = SDL_CreateTexture(
          app.renderer,
          SDL_PIXELFORMAT_RGBA32,
          SDL_TEXTUREACCESS_STATIC,
          app.imageWidth,
          app.imageHeight);
    }

    SDL_UpdateTexture(
        app.texture,
        nullptr,
        app.processedImage.data(),
        app.imageWidth * 4);
  }
}

static const SDL_DialogFileFilter filters[] = {
    { "BMP images", "bmp" },
    { "DG5 images", "dg5" },
    { "All images", "bmp;dg8" }
};

void SDLCALL OpenFileDialogCallback(void* userData, const char* const* fileList, int filter)
{
  if (!fileList || !*fileList) return;

  auto state = static_cast<AppState*>(userData);

  state->pendingOpenPath = *fileList;
  state->hasPendingOpen = true;
}

void SDLCALL SaveFileDialogCallback(void* userData, const char* const* fileList, int filter)
{
  if (!fileList || !*fileList) return;

  auto state = static_cast<AppState*>(userData);

  state->pendingSavePath = *fileList;
  state->hasPendingSave = true;
}

int main(int /*argc*/, char **/*argv*/)
{
  SDL_Init(SDL_INIT_VIDEO);

  float mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

  SDL_CreateWindowAndRenderer("Projekt", gApp.width, gApp.height, 0, &gApp.window, &gApp.renderer);

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

  ImGui_ImplSDL3_InitForSDLRenderer(gApp.window, gApp.renderer);
  ImGui_ImplSDLRenderer3_Init(gApp.renderer);

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

    if (gApp.hasPendingOpen) {
      gApp.hasPendingOpen = false;

      if (gApp.pendingOpenPath.extension() == ".bmp") {
         gApp.originalImage = LoadBMP(gApp.pendingOpenPath, gApp.imageWidth, gApp.imageHeight);
      } else if (gApp.pendingOpenPath.extension() == ".dg5") {
        fileManagement::DG5ImageData imageData = fileManagement::loadFromFile(gApp.pendingOpenPath);
        gApp.imageWidth = imageData.width;
        gApp.imageHeight = imageData.height;
        gApp.originalImage = std::vector<std::byte>(imageData.image.begin(), imageData.image.end());
      }

      if (gApp.texture) {
        SDL_DestroyTexture(gApp.texture);
        gApp.texture = nullptr;
      }

      ReprocessImage(gApp);
    }

    if (gApp.hasPendingSave) {
      gApp.hasPendingSave = false;

      if (gApp.pendingSavePath.extension() == ".bmp") {
        WriteBMP(gApp.pendingSavePath, gApp.processedImage, gApp.imageWidth, gApp.imageHeight);
      } else if (gApp.pendingSavePath.extension() == ".dg5") {
        fileManagement::saveToFile(
            gApp.processedImage, gApp.pendingSavePath, 
            gApp.imageWidth, gApp.imageHeight, 
            gApp.mode, gApp.dithering);
      }
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::DockSpace(ImGui::GetID("MainDock"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();

    windowFlags = ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Ustawienia", nullptr, windowFlags);
    {
      if (ImGui::Combo("Tryb", &gApp.mode,
            "Paleta Kolorowa Narzucona\0"
            "Odcienie Szarości Narzucone\0"
            "Paleta Kolorowa Dedykowana\0"
            "Odcienie Szarości Dedykowane\0")) {
        ReprocessImage(gApp);
      }

      if (ImGui::Combo("Dithering", &gApp.dithering,
            "Brak\0Bayer\0Floyd-Steinberg\0")) {
        ReprocessImage(gApp);
      }

      MyImGui::SettingsPalette(gApp.palette);

      if (ImGui::Button("Zapisz do pliku")) {
        SDL_ShowSaveFileDialog(SaveFileDialogCallback, &gApp, gApp.window, filters, 2, nullptr);
      }

      ImGui::SameLine();

      if (ImGui::Button("Odczytaj z pliku")) {
        SDL_ShowOpenFileDialog(OpenFileDialogCallback, &gApp, gApp.window, filters, 3, nullptr, 0);
      }

      ImGui::End();
    }


    ImGui::Begin("Obrazk");

    ImVec2 displaySize = ImVec2(gApp.imageWidth, gApp.imageHeight);
    MyImGui::Image((ImTextureID)gApp.texture, displaySize);

    ImGui::End();

    ImGui::Render();
    SDL_SetRenderDrawColor(gApp.renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(gApp.renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), gApp.renderer);
    SDL_RenderPresent(gApp.renderer);
  }

  SDL_Quit();
}
