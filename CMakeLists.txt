cmake_minimum_required(VERSION 3.22)
project(RayTracingWorkspace CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Crucial for Neovim (creates compile_commands.json for clangd LSP)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Find native Linux system dependencies
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads REQUIRED)

# 2. Replicate Include Directories from the Lua files
include_directories(
    Walnut/vendor/imgui
    Walnut/vendor/glfw/include
    Walnut/vendor/glm
    Walnut/vendor/stb_image
    Walnut/Walnut/src
    ${Vulkan_INCLUDE_DIRS}
)

# 3. Collect source files for the core engine (Walnut)
file(GLOB_RECURSE WALNUT_SOURCES 
    "Walnut/Walnut/src/**/*.cpp"
    "Walnut/Walnut/src/**/*.h"
)

# 4. Collect source files for your actual application (WalnutApp)
file(GLOB APP_ROOT_SOURCES
    "WalnutApp/src/*.cpp"
    "WalnutApp/src/*.h"
)

file(GLOB_RECURSE APP_SUB_SOURCES 
    "WalnutApp/src/**/*.cpp"
    "WalnutApp/src/**/*.h"
)

# Merge them together
set(APP_SOURCES ${APP_ROOT_SOURCES} ${APP_SUB_SOURCES})

# ImGui core source files (Walnut bundles them here)
file(GLOB IMGUI_SOURCES
    "Walnut/vendor/imgui/*.cpp"
    "Walnut/vendor/imgui/*.h"
)

# 5. Define target executable
add_executable(WalnutApp ${WALNUT_SOURCES} ${APP_SOURCES} ${IMGUI_SOURCES})

# 6. Link libraries natively (No Windows .lib files needed)
target_link_libraries(WalnutApp
    PRIVATE
    glfw
    Vulkan::Vulkan
    Threads::Threads
    dl
)
