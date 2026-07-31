cmake_minimum_required(VERSION 3.12)

set(LIGHT_PATH "${CMAKE_CURRENT_LIST_DIR}/../light_framework_mk3" CACHE PATH "path to the root directory of the light-framework project")
get_filename_component(LIGHT_PATH "${LIGHT_PATH}" REALPATH BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
include(${LIGHT_PATH}/external/light_preinit.cmake)

set(FONT_CRUSHER_PATH "${CMAKE_CURRENT_LIST_DIR}/../font-crusher" CACHE PATH "path to the root directory of the font-crusher project")
get_filename_component(FONT_CRUSHER_PATH "${FONT_CRUSHER_PATH}" REALPATH BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
option(REND_NO_FONT_CRUSHER "Disable use/requirement of the font-crusher (crush) tool" OFF)

project(librend C CXX ASM)

if(NOT TARGET light_core)
    add_subdirectory(${LIGHT_PATH}/ light_framework)
endif()

# Check crush (font-crusher) is available, building it as a host-native
# ExternalProject "utility" target if not already found -- mirrors how the
# Pico SDK lazily builds/finds picotool via pico_init_picotool()
function(rend_init_font_crusher)
    if (NOT TARGET crush AND NOT DEFINED crush_FOUND)
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake)
        find_package(crush MODULE)
    endif()

    if (TARGET crush AND NOT DEFINED ENV{_CRUSH_FOUND_THIS_RUN})
        # Internal environment variable to prevent printing multiple times
        set(ENV{_CRUSH_FOUND_THIS_RUN} 1)

        get_property(crush_location TARGET crush PROPERTY IMPORTED_LOCATION)
        message("Using crush (font-crusher) from ${crush_location}")
    endif()

    if (TARGET crush)
        set(crush_FOUND 1 PARENT_SCOPE)
    else()
        message("No crush (font-crusher) found")
    endif()
endfunction()

if(NOT REND_NO_FONT_CRUSHER)
    rend_init_font_crusher()
    if(crush_FOUND AND EXISTS "${FONT_CRUSHER_PATH}/cmake/CrushTools.cmake")
        # provides crush_invoke()/crush_add_font_target() etc. for turning font files
        # into rend_font_t sources at build time
        include(${FONT_CRUSHER_PATH}/cmake/CrushTools.cmake)
    endif()
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
    light_core
)
