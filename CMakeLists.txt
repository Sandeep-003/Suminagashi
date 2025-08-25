cmake_minimum_required(VERSION 3.5)
project(suminasashi)

set(CMAKE_CXX_STANDARD 17)
cmake_policy(SET CMP0054 NEW)

# Set policy to allow older CMake versions in dependencies
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

# Platform-specific settings for Emscripten
if(EMSCRIPTEN)
    set(PLATFORM "Web" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_GLFW=3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3")
endif()

include(FetchContent)

set(RAYLIB_VERSION 5.0)

# Set build options for raylib before fetching
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
if(EMSCRIPTEN)
    set(PLATFORM "Web" CACHE STRING "" FORCE)
endif()

find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    FetchContent_Declare(
        raylib
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_MakeAvailable(raylib)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")    

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sEXPORTED_FUNCTIONS=['_display','_clearCanvas','_main','_takeScreenshot'] -sEXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sEXPORTED_FUNCTIONS=\"['_display','_clearCanvas','_main','_takeScreenshot']\" -sEXPORTED_RUNTIME_METHODS=\"['ccall','cwrap']\"")

# Additional Emscripten flags for WebGL and screenshot support
if(EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMAX_WEBGL_VERSION=2 -sMIN_WEBGL_VERSION=1 -sFULL_ES2=1 -sOFFSCREENCANVAS_SUPPORT=1")
endif()

add_executable(suminasashi
    suminasashi.cpp 
    Drops.cpp 
    colors.cpp
)
target_compile_options(suminasashi PUBLIC "-std=c++20")
set_target_properties(suminasashi PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/web) 

set_target_properties(suminasashi PROPERTIES SUFFIX ".js")
target_link_options(suminasashi PUBLIC "-sUSE_GLFW=3" "-sMAX_WEBGL_VERSION=2" "-sMIN_WEBGL_VERSION=1" "-sFULL_ES2=1")
target_link_libraries(suminasashi raylib)