cmake_minimum_required(VERSION 3.21)

project(ImageFileFormatConverter VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ImageFileFormatConverter main.cpp)

add_subdirectory(SDL)
target_link_libraries(ImageFileFormatConverter PRIVATE SDL3::SDL3)

target_sources(ImageFileFormatConverter
  PRIVATE
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp

    imgui/backends/imgui_impl_sdl3.cpp

    imgui/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(ImageFileFormatConverter PUBLIC
    imgui
    imgui/backends
)

add_custom_command(
  TARGET ImageFileFormatConverter POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:ImageFileFormatConverter>
  VERBATIM
)
