cmake_minimum_required(VERSION 3.24)

add_executable(thunder_tests
    core/MathRandomTests.cpp
    game/GameWorldTests.cpp
    persistence/SaveTests.cpp
    assets/AssetCatalogTests.cpp
    "${CMAKE_SOURCE_DIR}/src/assets/AssetCatalog.cpp"
)
target_include_directories(thunder_tests PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(thunder_tests PRIVATE thunder_core GTest::gtest_main)
target_compile_definitions(thunder_tests PRIVATE
    TW_SOURCE_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)
tw_set_warnings(thunder_tests)

include(GoogleTest)
gtest_discover_tests(thunder_tests)
