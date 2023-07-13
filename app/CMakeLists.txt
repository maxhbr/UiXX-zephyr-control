# SPDX-License-Identifier: Apache-2.0

set(BOARD stamp_c3)
# set(BOARD esp32c3_devkitm)
 
set(CMAKE_C_FLAGS -lstdc++)
set(CMAKE_CXX_FLAGS -lstdc++)

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

zephyr_library_compile_options(-lstdc++ -std=c++17 -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-variable -Wno-comment)

FILE(GLOB app_sources src/*.cpp)
target_sources(app PRIVATE ${app_sources})
