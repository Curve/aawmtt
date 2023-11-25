cmake_minimum_required(VERSION 3.21)
project(aawmtt LANGUAGES CXX VERSION 2.5)

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

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -pedantic -pedantic-errors -Wfatal-errors)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unknown-warning-option -Wno-missing-field-initializers)

# -------------------------------------------------------------------------------------------------------------------------------
# Setup constants header
# -------------------------------------------------------------------------------------------------------------------------------

configure_file("constants.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/private/constants.hpp")

# -------------------------------------------------------------------------------------------------------------------------------
# Include headers
# -------------------------------------------------------------------------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PRIVATE "include" "private")

# -------------------------------------------------------------------------------------------------------------------------------
# Link required libraries
# -------------------------------------------------------------------------------------------------------------------------------

include("cmake/cpm.cmake")

CPMFindPackage(
  NAME              fmt
  GIT_TAG           10.1.1
  GIT_REPOSITORY    "https://github.com/fmtlib/fmt"
  EXCLUDE_FROM_ALL  ON
)

CPMFindPackage(
  NAME              spdlog
  VERSION           1.12.0
  GIT_REPOSITORY    "https://github.com/gabime/spdlog"
  EXCLUDE_FROM_ALL  ON
)

CPMFindPackage(
  NAME              range-v3
  GIT_TAG           0.12.0
  GIT_REPOSITORY    "https://github.com/ericniebler/range-v3"
  EXCLUDE_FROM_ALL  ON
)

CPMFindPackage(
  NAME              reproc
  GIT_TAG           1c07bdb
  GIT_REPOSITORY    "https://github.com/DaanDeMeyer/reproc"
  OPTIONS           "REPROC++ ON"
  EXCLUDE_FROM_ALL  ON
)

find_package(X11 REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE reproc++ fmt range-v3::meta spdlog::spdlog ${X11_LIBRARIES})

# -------------------------------------------------------------------------------------------------------------------------------
# Install executable
# -------------------------------------------------------------------------------------------------------------------------------

install(TARGETS ${PROJECT_NAME})
