cmake_minimum_required(VERSION 3.21)
project(tiny_docs_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Drogon CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)
pkg_check_modules(QPDF REQUIRED IMPORTED_TARGET libqpdf)

add_library(tiny_docs_core INTERFACE)
target_link_libraries(
  tiny_docs_core
  INTERFACE
    Drogon::Drogon
    nlohmann_json::nlohmann_json
    GTest::gtest
    PkgConfig::POPPLER_CPP
    PkgConfig::QPDF
)

add_executable(tiny-docs-server main.cpp)
target_link_libraries(tiny-docs-server PRIVATE tiny_docs_core)

enable_testing()
add_executable(tiny_docs_tests tests/smoke_test.cpp)
target_link_libraries(tiny_docs_tests PRIVATE GTest::gtest_main tiny_docs_core)

include(GoogleTest)
gtest_discover_tests(tiny_docs_tests)
