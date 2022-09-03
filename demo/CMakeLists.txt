
add_executable(rend_demo)

target_sources(rend_demo PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/main.c)

target_link_libraries(
        rend_demo PUBLIC
        rend
)
if(NOT PICO_NO_HARDWARE)
        pico_enable_stdio_semihosting(rend_demo 1)
        target_link_libraries(rend_demo PUBLIC pico_stdlib)
endif()
pico_add_extra_outputs(rend_demo)
