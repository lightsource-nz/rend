cmake_minimum_required(VERSION 3.12)
include(${LIGHT_PATH}/external/light_preinit.cmake)

project(librend C CXX ASM)

if(NOT TARGET light_framework)
    add_subdirectory(${LIGHT_PATH}/ light_framework)
endif()

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
    ${CMAKE_CURRENT_LIST_DIR}/src/rend_api.c
    ${CMAKE_CURRENT_LIST_DIR}/src/rend_draw.c
)

if(DEFINED REND_DEBUG_PIXEL_TRACE)
    target_compile_definitions(
        rend
        INTERFACE
        REND_DEBUG_PIXEL_TRACE
    )
endif()

if(DEFINED REND_DEBUG_API_TRACE)
    target_compile_definitions(
        rend
        INTERFACE
        REND_DEBUG_API_TRACE
    )
endif()

target_link_libraries(
    rend
    INTERFACE
    light_framework
)
