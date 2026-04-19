cmake_minimum_required(VERSION 3.20)
project(Lexico)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/vendor)

# -------------------------
# GLFW
# -------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.9
  SOURCE_DIR     ${CMAKE_SOURCE_DIR}/vendor/glfw
)

FetchContent_MakeAvailable(glfw)

# -------------------------
# ImGui
# -------------------------
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.4
  SOURCE_DIR     ${CMAKE_SOURCE_DIR}/vendor/imgui
)

FetchContent_Populate(imgui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
)

# -------------------------
# ImGui backends (GLFW + OpenGL3)
# -------------------------
add_library(imgui_backend STATIC
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_backend PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

# OpenGL
find_package(OpenGL REQUIRED)

target_link_libraries(imgui_backend
  PUBLIC imgui glfw OpenGL::GL
)

# -------------------------
# App
# -------------------------
add_executable(lexico-desktop gui/main.cpp)

target_link_libraries(lexico-desktop
  PRIVATE imgui imgui_backend glfw OpenGL::GL
)
