# Copyright 2025 Nuo Shen, Nanjing University
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(Thunder_Wings)

# Set the build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose build type: Debug or Release" FORCE)
endif()

# Set C++ standard to C++20 and make it required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Gather all source files recursively
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.cxx"
    "src/*.cc"
)

# Gather all header files recursively
file(GLOB_RECURSE HEADERS 
    "include/*.hpp"
    "include/*.h"
    "src/*.hpp"
    "src/*.h"
)

# Compiler-specific flags for GNU/Clang (Linux and MinGW on Windows)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        "$<$<CONFIG:Debug>:-g>"
        "$<$<CONFIG:Debug>:-fsanitize=address,undefined>"
        "$<$<CONFIG:Release>:-O3>"
        "$<$<CONFIG:Release>:-flto>"
    )
    
    add_link_options(
        "$<$<CONFIG:Debug>:-fsanitize=address,undefined>"
        "$<$<CONFIG:Release>:-flto>"
    )
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-fno-omit-frame-pointer)
    endif()
endif()

# Compiler-specific flags for MSVC (Windows)
if(MSVC)
    add_compile_options(
        "$<$<CONFIG:Debug>:/Zi>"
        "$<$<CONFIG:Debug>:/Od>"
        "$<$<CONFIG:Release>:/O2>"
        "$<$<CONFIG:Release>:/GL>"
    )
    add_link_options(
        "$<$<CONFIG:Release>:/LTCG>"
    )
endif()

# Find SFML package (version 2.5 or higher)
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

# Add the executable target
if(WIN32)
    set(ICON_RESOURCE_FILE "${CMAKE_SOURCE_DIR}/icon.rc")
    add_executable(thunder_wings WIN32 ${SOURCES} ${HEADERS} ${ICON_RESOURCE_FILE})
else()
    add_executable(thunder_wings ${SOURCES} ${HEADERS})
endif()

# Include directories for the target
target_include_directories(thunder_wings PRIVATE
    ${SFML_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Link SFML libraries to the target
target_link_libraries(thunder_wings PRIVATE
    sfml-audio
    sfml-graphics 
    sfml-window 
    sfml-system
)

if(WIN32)
    target_link_libraries(thunder_wings PRIVATE sfml-main)
endif()

# Define assets directories
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

# Windows-specific: Copy SFML DLLs and assets to executable directory
if(WIN32)
    # Get the directory where the executable is built
    set(EXECUTABLE_DIR $<TARGET_FILE_DIR:thunder_wings>)
    set(EXECUTABLE_PATH $<TARGET_FILE:thunder_wings>)
    
    # Get SFML DLL directory
    get_filename_component(SFML_DLL_DIR ${SFML_DIR} DIRECTORY)
    set(SFML_DLL_DIR "${SFML_DLL_DIR}/../../bin")
    
    # List of required DLLs (including openal32.dll)
    set(REQUIRED_DLLS
        "sfml-graphics-2.dll"
        "sfml-window-2.dll"
        "sfml-system-2.dll"
        "sfml-audio-2.dll"
        "openal32.dll"
    )
    
    # Copy SFML DLLs to executable directory
    add_custom_command(TARGET thunder_wings POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_DLL_DIR}/sfml-graphics-2.dll"
        "${SFML_DLL_DIR}/sfml-window-2.dll"
        "${SFML_DLL_DIR}/sfml-system-2.dll"
        "${SFML_DLL_DIR}/sfml-audio-2.dll"
        "${SFML_DLL_DIR}/openal32.dll"
        ${EXECUTABLE_DIR}
        COMMENT "Copying SFML DLLs and OpenAL to executable directory"
    )
    
    # Copy assets to executable directory (next to executable)
    set(TARGET_ASSETS_DIR "${EXECUTABLE_DIR}/assets")
    add_custom_command(TARGET thunder_wings POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TARGET_ASSETS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}" "${TARGET_ASSETS_DIR}"
        COMMENT "Copying assets to executable directory"
    )
else()
    # For non-Windows platforms, copy assets to build directory
    set(TARGET_ASSETS_DIR "${CMAKE_BINARY_DIR}/assets")
    add_custom_command(TARGET thunder_wings POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TARGET_ASSETS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}" "${TARGET_ASSETS_DIR}"
        COMMENT "Copying assets to build directory"
    )
endif()

# Optional: Install commands (commented out)
# install(TARGETS thunder_wings RUNTIME DESTINATION bin)
# install(DIRECTORY ${ASSETS_DIR} DESTINATION .)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Source files: ${SOURCES}")
message(STATUS "Assets directory: ${ASSETS_DIR} -> ${TARGET_ASSETS_DIR}")
if(WIN32)
    message(STATUS "SFML DLL directory: ${SFML_DLL_DIR}")
    message(STATUS "Executable directory: ${EXECUTABLE_DIR}")
endif()
