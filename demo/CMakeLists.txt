
add_executable(rend_demo)

target_sources(rend_demo PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/main.c
        ${CMAKE_CURRENT_LIST_DIR}/fonts/TypeLightSans_ttf_font.c
)
target_include_directories(rend_demo PRIVATE ${CMAKE_CURRENT_LIST_DIR}/fonts)

target_link_libraries(
        rend_demo PUBLIC
        rend
)
if(LIGHT_SYSTEM STREQUAL PICO_SDK AND LIGHT_PLATFORM STREQUAL TARGET)
        if(NOT PICO_NO_HARDWARE)
                pico_enable_stdio_semihosting(rend_demo 1)
                target_link_libraries(rend_demo PUBLIC pico_stdlib)
        endif()
        pico_add_extra_outputs(rend_demo)
endif()
