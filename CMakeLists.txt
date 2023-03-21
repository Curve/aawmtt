cmake_minimum_required(VERSION 3.24)
project(aawmtt LANGUAGES CXX VERSION 2.0)

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

include(FetchContent)

set(REPROC++ ON)
cmake_policy(SET CMP0135 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)

FetchContent_Declare(fmt        GIT_REPOSITORY "https://github.com/fmtlib/fmt"              GIT_TAG "9.1.0")
FetchContent_Declare(argparse   GIT_REPOSITORY "https://github.com/p-ranav/argparse"        GIT_TAG "e516556")
FetchContent_Declare(lockpp     GIT_REPOSITORY "https://github.com/Soundux/lockpp"          GIT_TAG "v1.0.2")
FetchContent_Declare(expected   GIT_REPOSITORY "https://github.com/TartanLlama/expected"    GIT_TAG "aa4f7a5")
FetchContent_Declare(reproc     GIT_REPOSITORY "https://github.com/DaanDeMeyer/reproc"      GIT_TAG "v14.2.4")

FetchContent_MakeAvailable(reproc argparse fmt expected lockpp)
target_link_libraries(${PROJECT_NAME} PRIVATE argparse reproc++ fmt tl::expected lockpp)

find_package(X11 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIR})

# -------------------------------------------------------------------------------------------------------------------------------
# Install targets
# -------------------------------------------------------------------------------------------------------------------------------

install(TARGETS ${PROJECT_NAME} DESTINATION bin)