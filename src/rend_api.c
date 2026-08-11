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
    trace_log_f("\"%s\": [%dx%d] @ %db", name, width, height, px_bits);
    return _context_create(name, width, height, px_bits);
}
void rend_context_set_font(rend_context_t *ctx, const rend_font_t *font)
{
    trace_log();
    _context_set_font(ctx, font);
}
void rend_context_set_rotation(rend_context_t *ctx, uint8_t rotation)
{
    trace_log_f("rotation=%d", rotation);
    _context_set_rotation(ctx, rotation);
}
void rend_context_set_flip(rend_context_t *ctx, uint8_t flip)
{
    trace_log_f("flip=%d", flip);
    _context_set_flip(ctx, flip);
}
void rend_context_enable_double_buffer(rend_context_t *ctx)
{
    trace_log();
    _context_enable_double_buffer(ctx);
}
bool rend_context_swap_buffers(rend_context_t *ctx)
{
    trace_log();
    return _context_swap_buffers(ctx);
}
void rend_draw_circle(const rend_context_t *ctx, rend_point2d centre, uint16_t radius, bool fill)
{
    trace_log_f("(%d,%d), radius=%d, fill=%d", centre.x, centre.y, radius, fill);
    _draw_circle(ctx, centre, radius, fill);
}
void rend_draw_arc(const rend_context_t *ctx, rend_point2d centre, uint16_t radius,
                   int16_t start_deg, int16_t end_deg)
{
    trace_log_f("(%d,%d), radius=%d, %d..%d deg", centre.x, centre.y, radius, start_deg, end_deg);
    _draw_arc(ctx, centre, radius, start_deg, end_deg);
}
void rend_draw_rect_rounded(const rend_context_t *ctx, rend_point2d p0, rend_point2d p1,
                            uint16_t radius, bool fill)
{
    trace_log_f("(%d,%d)->(%d,%d), radius=%d", p0.x, p0.y, p1.x, p1.y, radius);
    _draw_rect_rounded(ctx, p0, p1, radius, fill);
}
void rend_draw_point(const rend_context_t *ctx, rend_point2d p)
{
    trace_log_f("(%d,%d)", p.x, p.y);
    _draw_point(ctx, p);
}
void rend_draw_line(const rend_context_t *ctx, rend_point2d p0, rend_point2d p1, bool solid)
{
    trace_log_f("(%d,%d)->(%d,%d)", p0.x, p0.y, p1.x, p1.y);
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
    trace_log_f("(%d,%d): \"%s\"", p.x, p.y, text);
    _draw_text(ctx, p, text);
}
void rend_draw_rect(const rend_context_t *ctx,
                    rend_point2d p0, rend_point2d p1, bool fill)
{
    trace_log_f("(%d,%d)->(%d,%d)", p0.x, p0.y, p1.x, p1.y);
    _draw_rect(ctx, p0, p1, fill);
}
uint8_t *rend_buffer_to_string(const rend_context_t *ctx)
{
    trace_log();
    return _buffer_to_string(ctx);
}
void rend_debug_buffer_print_stdout(const rend_context_t *ctx)
{
    trace_log();
    _buffer_print_stdout(ctx);
}
