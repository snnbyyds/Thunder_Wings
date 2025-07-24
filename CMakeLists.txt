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
cmake_minimum_required(VERSION 3.15)

# Project name and version
project(Thunder_Wings VERSION 1.0.0)

# Enforce policies
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# Default build type to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose build type: Debug or Release" FORCE)
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure flags per compiler and configuration
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Debug flags
    set(_compiler_options_debug "-O1;-g;-fsanitize=address,undefined;-fno-omit-frame-pointer;-Wall")
    set(_linker_options_debug "-fsanitize=address,undefined")
    # Release flags
    set(_compiler_options_release "-O3;-flto;-Wall")
    set(_linker_options_release "-flto")
elseif(MSVC)
    # Debug flags
    set(_compiler_options_debug "/O1;/Zi;/fsanitize=address;/Zo;/Wall")
    set(_linker_options_debug "/DEBUG")
    # Release flags
    set(_compiler_options_release "/O2;/GL;/Wall")
    set(_linker_options_release "/LTCG")
endif()

# Set current build options for config.h
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    string(REPLACE ";" " " _TW_COMPILER_OPTIONS "${_compiler_options_debug}")
    string(REPLACE ";" " " _TW_LINKER_OPTIONS "${_linker_options_debug}")
else()
    string(REPLACE ";" " " _TW_COMPILER_OPTIONS "${_compiler_options_release}")
    string(REPLACE ";" " " _TW_LINKER_OPTIONS "${_linker_options_release}")
endif()

# Configure config.h
configure_file(
    ${CMAKE_SOURCE_DIR}/config.h.in
    ${CMAKE_BINARY_DIR}/config.h
    @ONLY
)
include_directories(${CMAKE_BINARY_DIR})

# Boost configuration
set(Boost_MINIMUM_VERSION 1.75)

# Set static linking for Windows
if(WIN32)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
    
    add_definitions(-DBOOST_LOG_STATIC_LINK)
    add_definitions(-DBOOST_ALL_NO_LIB)
    
    # MSVC specific settings
    if(MSVC)
        add_definitions(-D_WIN32_WINNT=0x0A00)
        add_definitions(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
        add_definitions(-D_SCL_SECURE_NO_WARNINGS)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    endif()
else()
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREADED ON)
endif()

# Find Boost with required components for logging
find_package(Boost ${Boost_MINIMUM_VERSION} REQUIRED COMPONENTS 
    json 
    log 
    log_setup 
    system 
    filesystem 
    thread
)

if(NOT Boost_LOG_FOUND)
    message(FATAL_ERROR "Boost.Log library not found. Please check your Boost installation.")
endif()

# Gather sources and headers
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.cxx" "${CMAKE_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/include/*.hpp" "${CMAKE_SOURCE_DIR}/include/*.h" "${CMAKE_SOURCE_DIR}/src/*.hpp" "${CMAKE_SOURCE_DIR}/src/*.h")

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

# Executable target
if(WIN32)
    set(ICON_RESOURCE_FILE "${CMAKE_SOURCE_DIR}/icon.rc")
    add_executable(thunder_wings WIN32 ${SOURCES} ${HEADERS} ${ICON_RESOURCE_FILE})
else()
    add_executable(thunder_wings ${SOURCES} ${HEADERS})
endif()

# Apply compile and linker options using generator expressions
target_compile_options(thunder_wings PRIVATE 
    $<$<CONFIG:Debug>:${_compiler_options_debug}>
    $<$<CONFIG:Release>:${_compiler_options_release}>
)
target_link_options(thunder_wings PRIVATE 
    $<$<CONFIG:Debug>:${_linker_options_debug}>
    $<$<CONFIG:Release>:${_linker_options_release}>
)

# Include dirs
target_include_directories(thunder_wings PRIVATE
    ${SFML_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Link libraries - Core dependencies
target_link_libraries(thunder_wings PRIVATE
    sfml-audio
    sfml-graphics
    sfml-window
    sfml-system
    Boost::json
)

# Link Boost.Log and its dependencies
target_link_libraries(thunder_wings PRIVATE
    ${Boost_LOG_LIBRARY}
    ${Boost_LOG_SETUP_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
)

# Windows specific libraries
if(WIN32)
    target_link_libraries(thunder_wings PRIVATE 
        sfml-main
        ws2_32     # Required for Boost.Log on Windows
        advapi32   # Required for Boost.Log on Windows
        ole32
        oleaut32
        uuid
    )
else()
    target_link_libraries(thunder_wings PRIVATE pthread)
endif()

# Assets directory
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

# Post-build commands: copy assets and DLLs
if(WIN32)
    set(EXECUTABLE_DIR $<TARGET_FILE_DIR:thunder_wings>)
    get_filename_component(SFML_BIN_DIR ${SFML_DIR} DIRECTORY)
    string(APPEND SFML_BIN_DIR "/../../bin")
    set(DLLS "sfml-graphics-2.dll" "sfml-window-2.dll" "sfml-system-2.dll" "sfml-audio-2.dll" "openal32.dll")
    foreach(DLL ${DLLS})
        add_custom_command(TARGET thunder_wings POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_BIN_DIR}/${DLL}" "${EXECUTABLE_DIR}"
        )
    endforeach()
    add_custom_command(TARGET thunder_wings POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_DIR} ${EXECUTABLE_DIR}/assets
    )
else()
    add_custom_command(TARGET thunder_wings POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_DIR} ${CMAKE_BINARY_DIR}/assets
    )
endif()

# Build info
message(STATUS "Thunder_Wings ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler options: ${_TW_COMPILER_OPTIONS}")
message(STATUS "Linker options:   ${_TW_LINKER_OPTIONS}")
message(STATUS "Assets directory: ${ASSETS_DIR}")
if(WIN32)
    message(STATUS "SFML DLL directory: ${SFML_BIN_DIR}")
    message(STATUS "Executable directory: ${EXECUTABLE_DIR}")
endif()

# Installation rules
install(TARGETS thunder_wings
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${ASSETS_DIR}/
    DESTINATION assets
)
