/*
 *  rend.c
 *  routines for managing and drawing to image buffers
 * 
 *  authored by Alex Fulton
 *  created august 2022
 * 
 */

/*
 *  --> TODO: implement optimized methods for filling horizontal spans of
 *      pixels in memory simultaneously
 *      [https://github.com/lightsource-nz/rend/issues/2]
 *  --> TODO: implement mirroring, rotation and other linear transforms
 *      [https://github.com/lightsource-nz/rend/issues/3]
 *  --> TODO: implement support for grayscale and colour pixel formats
 */

#include <rend.h>
#include "rend_internal.h"

#include <stdio.h>
#include <string.h>


static void _module_event(const struct light_module *module, uint8_t event, void *arg);
Light_Module_Define(rend, _module_event, &light_core);

static void _event_load(const struct light_module *module)
{
    
}
static void _event_unload(const struct light_module *module)
{
        
}
static void _module_event(const struct light_module *module, uint8_t event, void *arg)
{
        switch(event) {
        case LF_EVENT_MODULE_LOAD:
                _event_load(module);
                break;
        case LF_EVENT_MODULE_UNLOAD:
                _event_unload(module);
                break;
        }
}
