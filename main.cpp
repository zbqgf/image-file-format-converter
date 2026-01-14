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
#define STBI_ONLY_BMP
#include "stb_image.h"

#include "MyImGui.h"
#include "Palette.h"
#include "Quantization.h"
#include "Dithering.h"

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
};

static AppState gApp;

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
    //{ "DG8 images", "dg8" },
    //{ "All images", "bmp;dg8" }
};

void SDLCALL OpenFileDialogCallback(void* userData, const char* const* fileList, int filter)
{
  if (!fileList || !*fileList) return;

  auto state = static_cast<AppState*>(userData);

  state->pendingOpenPath = *fileList;
  state->hasPendingOpen = true;
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

      gApp.originalImage = LoadFromFile(gApp.pendingOpenPath, gApp.imageWidth, gApp.imageHeight);

      if (gApp.texture) {
        SDL_DestroyTexture(gApp.texture);
        gApp.texture = nullptr;
      }

      ReprocessImage(gApp);
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

    ImGui::BeginMenuBar();

    if (ImGui::BeginMenu("Menu")) {
      if (ImGui::MenuItem("Otwórz")) {
        SDL_ShowOpenFileDialog(OpenFileDialogCallback, &gApp, gApp.window, filters, 1, nullptr, 0);
      }

      if (ImGui::MenuItem("Zapisz jako")) {
      }

      ImGui::Separator();

      if (ImGui::MenuItem("O programie")) {
      }

      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();

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
    }
    ImGui::End();

    ImGui::Begin("obrazek1.bmp");

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
