cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)

project(librend C CXX ASM)

pico_sdk_init()

add_subdirectory(demo)

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

if(DEFINED REND_RENDER_DEBUG)
    target_compile_definitions(
        rend
        INTERFACE
        REND_DEBUG_DISPLAY_PIXEL
    )
endif()

if(DEFINED REND_API_TRACE)
    target_compile_definitions(
        rend
        INTERFACE
        REND_API_TRACE
    )
endif()

target_link_libraries(
    rend
    INTERFACE
)
