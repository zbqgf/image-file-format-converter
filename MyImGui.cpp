#include "MyImGui.h"

namespace
{
  constexpr ImVec2 kDefaultUV0(0.0f, 0.0f);
  constexpr ImVec2 kDefaultUV1(1.0f, 1.0f);
  constexpr ImVec4 kDefaultBackgroundColor(0, 0, 0, 0);
  constexpr ImVec4 kDefaultTintColor(1, 1, 1, 1);
}

void MyImGui::Image(ImTextureRef texRef, ImVec2 displaySize)
{
  Image(texRef, displaySize, kDefaultUV0, kDefaultUV1,
      kDefaultBackgroundColor, kDefaultTintColor);
}

void MyImGui::Image(
    ImTextureRef texRef, ImVec2 displaySize,
    ImVec2 uv0, ImVec2 uv1)
{
  Image(texRef, displaySize, kDefaultUV0, kDefaultUV1,
      kDefaultBackgroundColor, kDefaultTintColor);
}

void MyImGui::Image(
    ImTextureRef texRef, ImVec2 imageSize,
    ImVec2 uv0, ImVec2 uv1,
    ImVec4 bgColor, ImVec4 tintColor)
{
  // Check image size
  if (imageSize.x <= 0.0f || imageSize.y <= 0.0f) {
    // Invalid size, do nothing
    return;
  }

  ImGui::BeginChild("ImageRegion", ImVec2(0,0), false, ImGuiWindowFlags_NoMove);

  // Get texture size
  ImVec2 textureSize = imageSize;
  if (textureSize.x <= 0.0f || textureSize.y <= 0.0f) {
    // use image size as texture size
    textureSize = ImVec2(
        imageSize.x / std::abs(uv1.x - uv0.x),
        imageSize.y / std::abs(uv1.y - uv0.y));
  }

  // Respect the image aspect ratio
  ImVec2 widgetSize = ImGui::GetContentRegionAvail();
  ImVec2 displaySize = widgetSize;
  const float aspectRatio = textureSize.x / textureSize.y;
  if (displaySize.x / displaySize.y > aspectRatio) {
    displaySize.x = displaySize.y * aspectRatio;
  } else {
    displaySize.y = displaySize.x / aspectRatio;
  }

  // Center the image
  ImVec2 displayPos = ImVec2(
      (widgetSize.x - displaySize.x) * 0.5f,
      (widgetSize.y - displaySize.y) * 0.5f);

  // Set the display position
  ImGui::SetCursorPos(displayPos);

  // Display the texture
  ImGui::Image(texRef, displaySize, uv0, uv1, tintColor, bgColor);

  ImGui::EndChild();
}

void MyImGui::SettingsPalette(std::span<uint32_t> palette)
{
  ImGui::Text("Paleta");
  ImGui::BeginGroup();

  const float windowVisibleX2 =
    ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;

  for (int i = 0; i < palette.size(); i++) {
    ImGui::PushID(i);

    ImGuiColorEditFlags colorButtonFlags =
      ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker;

    ImVec4 color = ImGui::ColorConvertU32ToFloat4(palette[i]);
    ImGui::ColorButton("##palette", color, colorButtonFlags, ImVec2(32, 32));

    float lastButtonX2 = ImGui::GetItemRectMax().x;
    float nextButtonX2 = lastButtonX2 + ImGui::GetStyle().ItemSpacing.y + 32;
    if (nextButtonX2  < windowVisibleX2) {
      ImGui::SameLine();
    }

    ImGui::PopID();
  }
  ImGui::EndGroup();
}
