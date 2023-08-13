#include<rend.h>
#include<stdio.h>

#ifdef __HAVE_RP2_HW
#   include <pico/stdio.h>
#endif

void rend_demo_event(const struct light_module *module, uint8_t event);
static uint8_t rend_demo_main(struct light_application *app);

Light_Application_Define(rend_demo, rend_demo_event, rend_demo_main,
                                &rend,
                                &light_framework);

int main(int argc, char *argv[])
{
        light_framework_init();
        light_framework_run();

        return LIGHT_OK;
}

void rend_demo_event(const struct light_module *module, uint8_t event)
{
        switch(event) {
                case LF_EVENT_LOAD:
                break;
                // TODO implement unregister for event hooks
                case LF_EVENT_UNLOAD:
                break; 
        }
}
static uint8_t rend_demo_main(struct light_application *app)
{
#ifdef __HAVE_RP2_HW
        stdio_init_all();
#endif
        rend_context_t *ctx = rend_context_create("rend_demo", 20, 10, 1);
        ctx->point_radius = 2;

        rend_draw_point(ctx, (rend_point2d) {5,5});
        rend_draw_line(ctx, (rend_point2d) {8,5}, (rend_point2d) {14,7}, true);

        rend_debug_buffer_print_stdout(ctx);
    
        return LF_STATUS_SHUTDOWN;
}
