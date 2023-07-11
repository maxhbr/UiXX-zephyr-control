# SPDX-License-Identifier: Apache-2.0

set(BOARD stamp_c3) # set(BOARD xiao_ble)
# set(SHIELD adafruit_2_8_tft_touch_v2)
 
set(CMAKE_C_FLAGS -lstdc++)
set(CMAKE_CXX_FLAGS -lstdc++)

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)


FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)

generate_inc_file_for_target(
    app
    src/https-cert.der
    ${gen_dir}/https-cert.der.inc
    )
