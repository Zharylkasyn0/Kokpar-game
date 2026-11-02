cmake_minimum_required(VERSION 3.20)
project(KokparWobblySteppe LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
include(FetchContent)

# GLFW (windowing/input) - built from source, no extras.
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)

# GLM (header-only math)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(glfw glm)

# GLAD (OpenGL loader) - generated files must be placed manually, see README.md.
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad)
if(NOT EXISTS ${GLAD_DIR}/src/glad.c OR NOT EXISTS ${GLAD_DIR}/include/glad/glad.h)
    message(FATAL_ERROR
        "GLAD files not found.\n"
        "Generate GLAD 1.x (gl 3.3, Core profile, no extensions) and copy:\n"
        "  third_party/glad/include/glad/glad.h\n"
        "  third_party/glad/include/KHR/khrplatform.h\n"
        "  third_party/glad/src/glad.c\n"
        "See README.md for details.")
endif()

add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

# ---------------------------------------------------------------------------
# Game executable
# ---------------------------------------------------------------------------
add_executable(kokpar
    src/main.cpp
    src/Game.cpp
    src/Input.cpp
    src/Shader.cpp
    src/Mesh.cpp
    src/Assets.cpp
    src/Renderer.cpp
    src/Camera.cpp
    src/World.cpp
    src/Horse.cpp
    src/Goat.cpp
    src/Hud.cpp
)

# Note: do not add 'src' to target_include_directories; on case-insensitive filesystems (Windows),
# MSVC's <cmath> does #include <math.h>, which would mistakenly resolve to src/Math.h instead of UCRT math.h.
# All sources in src/ use quoted relative includes ("...") which resolve within src/.
target_link_libraries(kokpar PRIVATE glad glfw glm::glm)
target_compile_definitions(kokpar PRIVATE GLFW_INCLUDE_NONE GLM_FORCE_RADIANS NOMINMAX)

if(MSVC)
    target_compile_options(kokpar PRIVATE /W4 /utf-8)
else()
    target_compile_options(kokpar PRIVATE -Wall -Wextra)
endif()

# Make Visual Studio start-up project / working directory sane.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT kokpar)
