cmake_minimum_required(VERSION 3.12)
#include(pico_sdk_import.cmake)

project(librend C CXX ASM)

#pico_sdk_init()

add_library(rend INTERFACE)

target_include_directories(
    rend
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(
    rend
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/rend.c
)

target_link_libraries(
    rend
    INTERFACE
)

