cmake_minimum_required(VERSION 3.24)
project(aawmtt LANGUAGES CXX VERSION 2.4)

# -------------------------------------------------------------------------------------------------------------------------------
# We only support UNIX platforms
# -------------------------------------------------------------------------------------------------------------------------------

if (NOT UNIX)
    message(FATAL "[aawmtt] This program is not supported on non unix platforms")
endif()

# -------------------------------------------------------------------------------------------------------------------------------
# Setup library                          
# -------------------------------------------------------------------------------------------------------------------------------

file(GLOB src "src/*.cpp")
add_executable(${PROJECT_NAME} ${src})

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-missing-field-initializers)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -pedantic -pedantic-errors -Wfatal-errors)
endif()

# -------------------------------------------------------------------------------------------------------------------------------
# Setup constants header
# -------------------------------------------------------------------------------------------------------------------------------

configure_file("constants.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/include/constants.hpp")

# -------------------------------------------------------------------------------------------------------------------------------
# Include headers
# -------------------------------------------------------------------------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PRIVATE "include")

# -------------------------------------------------------------------------------------------------------------------------------
# Link required libraries
# -------------------------------------------------------------------------------------------------------------------------------

include("cmake/cpm.cmake")

set(REPROC++ ON)

CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:CLIUtils/CLI11@2.3.2")
CPMAddPackage("gh:gabime/spdlog@1.11.0")
CPMAddPackage("gh:DaanDeMeyer/reproc#1c07bdb")

target_link_libraries(${PROJECT_NAME} PRIVATE CLI11::CLI11 reproc++ fmt spdlog::spdlog)

find_package(X11 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIR})

# -------------------------------------------------------------------------------------------------------------------------------
# Install targets
# -------------------------------------------------------------------------------------------------------------------------------

install(TARGETS ${PROJECT_NAME} DESTINATION bin)