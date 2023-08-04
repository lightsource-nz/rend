/*
 *  rend_api.c
 *  dispatch layer for rend API functions
 * 
 *  authored by Alex Fulton
 *  created august 2022
 * 
 */

#include <rend.h>
#include "rend_internal.h"

#include <stdio.h>
#include <string.h>

rend_context_t *rend_context_create(const uint8_t *name, uint16_t width, uint16_t height, uint8_t px_bits)
{
    trace_log("\"%s\": [%dx%d] @ %db", width, height, px_bits);
    _context_create(name, width, height, px_bits);
}
void rend_draw_circle(const rend_context_t *ctx, rend_point2d centre, uint16_t radius, bool fill)
{
    trace_log("(%d,%d), radius=%d", centre.x, centre.y);
    _draw_circle(ctx, centre, radius, fill);
}
void rend_draw_point(const rend_context_t *ctx, rend_point2d p)
{
    trace_log("(%d,%d)", p.x, p.y);
    _draw_point(ctx, p);
}
void rend_draw_line(const rend_context_t *ctx, rend_point2d p0, rend_point2d p1, bool solid)
{
    trace_log("(%d,%d)->(%d,%d)", p0.x, p0.y, p1.x, p1.y);
    _draw_line(ctx, p0, p1, solid);
}
void rend_draw_clear(const rend_context_t *ctx)
{
    trace_log();
    _draw_clear(ctx);
}
void rend_draw_text(const rend_context_t *ctx,
                    rend_point2d p, const uint8_t *text)
{
    trace_log("(%d,%d): \"%s\"", p.x, p.y, text);
    _draw_text(ctx, p, text);
}
void rend_draw_rect(const rend_context_t *ctx,
                    rend_point2d p0, rend_point2d p1, bool fill)
{
    trace_log("(%d,%d)->(%d,%d)", p0.x, p0.y, p1.x, p1.y);
    _draw_rect(ctx, p0, p1, fill);
}
uint8_t *rend_buffer_to_string(const rend_context_t *ctx)
{
    trace_log();
    _buffer_to_string(ctx);
}
void rend_debug_buffer_print_stdout(const rend_context_t *ctx)
{
    trace_log();
    _buffer_print_stdout(ctx);
}
