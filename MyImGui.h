#pragma once

#include <cstdint>
#include <span>

#include "imgui.h"


namespace MyImGui
{

  IMGUI_API void Image(ImTextureRef texRef, ImVec2 displaySize);

  IMGUI_API void Image(
      ImTextureRef texRef, ImVec2 displaySize,
      ImVec2 uv0, ImVec2 uv1);

  IMGUI_API void Image(
      ImTextureRef texRef, ImVec2 imageSize,
      ImVec2 uv0, ImVec2 uv1,
      ImVec4 bgColor, ImVec4 tintColor);

  void SettingsPalette(std::span<std::uint32_t> palette);

} //MyImGui
