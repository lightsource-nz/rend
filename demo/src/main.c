#include<rend.h>
#include<stdio.h>

#ifdef PICO_RP2040
#   include <pico/stdio.h>
#endif
//#include <pico/stdlib.h>

int main(int argc, char *argv[])
{
#ifdef PICO_RP2040
    stdio_init_all();
#endif
    rend_context_t *ctx = rend_context_create(20, 10, 1);
    ctx->point_radius = 2;

    rend_draw_point(ctx, (rend_point2d) {5,5});
    rend_draw_line(ctx, (rend_point2d) {8,5}, (rend_point2d) {14,7}, true);

    rend_debug_buffer_print_stdout(ctx);
    
}
