cmake_minimum_required(VERSION 3.24)

project(ThunderWings VERSION 2.0.0 LANGUAGES CXX)

include(CTest)
include(FetchContent)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TW_BUILD_TESTS "Build the GoogleTest suite" ON)
option(TW_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(TW_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address,undefined)
    else()
        message(WARNING "TW_ENABLE_SANITIZERS is currently supported only by GCC and Clang")
    endif()
endif()

function(tw_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(SFML 3 COMPONENTS Graphics Window Audio REQUIRED)
if(WIN32)
    # SFML::Main supplies WinMain so a standard main() works with WIN32_EXECUTABLE.
    # It is optional so a console build still works with unusual SFML packages.
    find_package(SFML 3 COMPONENTS Main QUIET)
endif()

find_package(nlohmann_json 3.11 CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found; fetching a pinned release")
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/archive/refs/tags/v3.12.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

if(TW_BUILD_TESTS AND BUILD_TESTING)
    find_package(GTest CONFIG QUIET)
    if(NOT GTest_FOUND)
        message(STATUS "GoogleTest not found; fetching a pinned release")
        set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(googletest)
    endif()
endif()

# ---------------------------------------------------------------------------
# Generated build information
# ---------------------------------------------------------------------------

file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}/assets" TW_SOURCE_ASSETS)
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/BuildConfig.hpp.in"
    "${CMAKE_BINARY_DIR}/generated/thunder/BuildConfig.hpp"
    @ONLY
)

# ---------------------------------------------------------------------------
# Core simulation and persistence library (no SFML dependency)
# ---------------------------------------------------------------------------

add_library(thunder_core STATIC
    src/core/Log.cpp
    src/core/Random.cpp
    src/game/GameWorld.cpp
    src/game/WorldSpawning.cpp
    src/game/WorldCollisions.cpp
    src/persistence/SaveCodec.cpp
    src/persistence/SaveRepository.cpp
)
target_include_directories(thunder_core PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(thunder_core PRIVATE nlohmann_json::nlohmann_json)
tw_set_warnings(thunder_core)

# ---------------------------------------------------------------------------
# Game executable (SFML 3)
# ---------------------------------------------------------------------------

add_executable(thunder_wings
    src/main.cpp
    src/app/Application.cpp
    src/app/Cli.cpp
    src/assets/AssetCatalog.cpp
    src/assets/AssetManager.cpp
    src/render/GameRenderer.cpp
    src/render/MenuRenderer.cpp
)
target_include_directories(thunder_wings PRIVATE "${CMAKE_BINARY_DIR}/generated")
target_link_libraries(thunder_wings PRIVATE
    thunder_core
    SFML::Graphics
    SFML::Audio
)
tw_set_warnings(thunder_wings)

if(WIN32)
    target_sources(thunder_wings PRIVATE icon.rc)
    if(TARGET SFML::Main)
        target_link_libraries(thunder_wings PRIVATE SFML::Main)
        set_target_properties(thunder_wings PROPERTIES WIN32_EXECUTABLE TRUE)
    else()
        message(WARNING "SFML::Main is unavailable; building a console Windows application")
    endif()

    foreach(dependency SFML::System SFML::Window SFML::Graphics SFML::Audio)
        if(TARGET ${dependency})
            add_custom_command(TARGET thunder_wings POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:${dependency}>"
                    "$<TARGET_FILE_DIR:thunder_wings>"
            )
        endif()
    endforeach()
endif()

add_custom_command(TARGET thunder_wings POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:thunder_wings>/assets"
)

install(TARGETS thunder_wings RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY assets/ DESTINATION "${CMAKE_INSTALL_DATADIR}/thunder-wings/assets")

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

if(TW_BUILD_TESTS AND BUILD_TESTING)
    add_subdirectory(tests)
endif()

message(STATUS "Thunder Wings ${PROJECT_VERSION}")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "SFML version:      ${SFML_VERSION}")
message(STATUS "Assets:            ${CMAKE_SOURCE_DIR}/assets")
message(STATUS "Tests:             ${TW_BUILD_TESTS}")
message(STATUS "Sanitizers:        ${TW_ENABLE_SANITIZERS}")
