cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)

project(librend C CXX ASM)

pico_sdk_init()

add_library(rend)

target_include_directories(
    rend
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(
    rend
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/rend.c
)

target_link_libraries(
    rend
    PUBLIC
    shale_core
    class_display
    driver_sh1007
)

