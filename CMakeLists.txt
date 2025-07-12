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

cmake_minimum_required(VERSION 3.10)

project(Thunder_Wings)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose build type: Debug or Release" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.cxx"
    "src/*.cc"
)

file(GLOB_RECURSE HEADERS 
    "include/*.hpp"
    "include/*.h"
    "src/*.hpp"
    "src/*.h"
)

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

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

add_executable(thunder_wings ${SOURCES} ${HEADERS})

target_include_directories(thunder_wings PRIVATE
    ${SFML_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(thunder_wings PRIVATE
    sfml-graphics 
    sfml-window 
    sfml-system
)

set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
set(TARGET_ASSETS_DIR ${CMAKE_BINARY_DIR}/assets)

add_custom_command(TARGET thunder_wings POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TARGET_ASSETS_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_DIR} ${TARGET_ASSETS_DIR}
    COMMENT "Copying assets to build directory"
)

# install(TARGETS thunder_wings RUNTIME DESTINATION bin)
# install(DIRECTORY ${ASSETS_DIR} DESTINATION .)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Source files: ${SOURCES}")
message(STATUS "Assets directory: ${ASSETS_DIR} -> ${TARGET_ASSETS_DIR}")
