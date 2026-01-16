cmake_minimum_required(VERSION 3.15)
project(GreyScottSim VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# macOS: Ensure the correct SDK is used and C++ stdlib is found
if(APPLE)
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(CMAKE_OSX_SYSROOT "${MACOS_SDK_PATH}")
    # Workaround: some Command Line Tools installations have broken c++ header paths
    # Force include the SDK's C++ headers
    include_directories(SYSTEM "${MACOS_SDK_PATH}/usr/include/c++/v1")
endif()

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages (system packages)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# SDL2 - try CONFIG mode first (vcpkg/CMake package), fall back to MODULE mode (system)
find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED)
endif()

# ImGui - try system package, otherwise we use vendored sources in src/
find_package(imgui CONFIG QUIET)
if(imgui_FOUND)
    message(STATUS "Using system imgui package")
    set(IMGUI_TARGET imgui::imgui)
else()
    message(STATUS "Using vendored imgui (sources in src/)")
    set(IMGUI_TARGET "")
endif()

# OpenCL is optional (not available/deprecated on macOS)
option(USE_OPENCL "Enable OpenCL GPU compute" ON)
if(APPLE)
    set(USE_OPENCL OFF CACHE BOOL "OpenCL disabled on macOS" FORCE)
    message(STATUS "OpenCL disabled on macOS (deprecated by Apple)")
endif()

if(USE_OPENCL)
    find_package(OpenCL REQUIRED)
    add_compile_definitions(USE_OPENCL)
    message(STATUS "OpenCL GPU compute: ENABLED")
else()
    message(STATUS "OpenCL GPU compute: DISABLED (CPU-only mode)")
endif()

# Collect source files
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Handle SDL2 include directories (different between CONFIG and MODULE mode)
if(TARGET SDL2::SDL2)
    # CONFIG mode (vcpkg or modern CMake package)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        SDL2::SDL2
    )
else()
    # MODULE mode (system package with SDL2_INCLUDE_DIRS/SDL2_LIBRARIES)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
endif()

# Link remaining libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    GLEW::GLEW
    ${IMGUI_TARGET}
    $<$<BOOL:${USE_OPENCL}>:OpenCL::OpenCL>
    $<$<PLATFORM_ID:Linux>:dl>
)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()

# Copy runtime resources to build directory (if they exist)
if(EXISTS ${CMAKE_SOURCE_DIR}/kernels)
    file(COPY ${CMAKE_SOURCE_DIR}/kernels DESTINATION ${CMAKE_BINARY_DIR})
endif()
if(EXISTS ${CMAKE_SOURCE_DIR}/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Print configuration info
message(STATUS "")
message(STATUS "Grey-Scott Simulation Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  OpenGL: Found")
if(USE_OPENCL)
    message(STATUS "  OpenCL: ${OpenCL_LIBRARIES}")
else()
    message(STATUS "  OpenCL: Disabled")
endif()
message(STATUS "")
