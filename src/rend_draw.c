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
 *  pixels in memory simultaneously
 *  --> TODO: implement mirroring, rotation and other linear transforms
 *  --> TODO: implement support for grayscale and colour pixel formats
 */

#include <rend.h>
#include "rend_internal.h"

#include <stdio.h>
#include <string.h>

// TODO implement image transforms
// TODO implement for BPP values > 1
void _set_pixel(const rend_context_t *ctx, rend_point2d p, uint32_t color)
{
#ifdef REND_DEBUG_PIXEL_TRACE
    trace_log_f("(%d,%d) = %d", p.x, p.y, color);
#endif
    // resolve pixel address in buffer, after 2d transforms
    if(ctx->px_bits == 1) {     // monochrome image: 8px per byte
        uint8_t width_bytes = (ctx->dim_x / 8) + ((ctx->dim_x % 8)? 1 : 0);
        uint8_t *buf_byte = &ctx->buffer[p.y * width_bytes + p.x / 8];
        if(color) {
            *buf_byte = *buf_byte | (1 << p.x % 8);
        } else {
            *buf_byte = *buf_byte & ~(1 << p.x % 8);
        }
    }
/*
#ifdef REND_DEBUG_DISPLAY_PIXEL
    rend_debug_buffer_print_stdout(ctx);
#endif
*/
}

uint32_t _get_pixel(const rend_context_t *ctx, rend_point2d p)
{
    if(ctx->px_bits == 1) {     // monochrome image: 8px per byte
        uint8_t width_bytes = (ctx->dim_x / 8) + ((ctx->dim_x % 8)? 1 : 0);
        //return ctx->buffer[p.y * width_bytes + p.x / 8] & (0x80 >> p.x % 8);
        uint8_t px_block = ctx->buffer[p.y * width_bytes + p.x / 8];
        uint32_t out = px_block & (1 << p.x % 8);
        return out;
    }
    return 0;
}

// TODO add optimized version to fill all pixels along horizontal spans using memset
void _set_octant_pixels(const rend_context_t *ctx, rend_point2d centre, rend_point2d p, uint32_t color)
{
    // iterate through octants in clockwise order
    _set_pixel(ctx, (rend_point2d) { centre.x + p.x, centre.y + p.y }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x + p.y, centre.y + p.x }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x + p.y, centre.y - p.x }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x + p.x, centre.y - p.y }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x - p.x, centre.y - p.y }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x - p.y, centre.y - p.x }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x - p.y, centre.y + p.x }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x - p.x, centre.y + p.y }, color);
}
void _set_pixels_circle(const rend_context_t *ctx, rend_point2d centre, uint8_t radius, uint32_t color)
{
    int16_t d = 3 - 2 * radius;
    rend_point2d p = {0, radius};

    _set_octant_pixels(ctx, centre, p, color);
    while(p.y >= p.x) {
        p.x++;
        if(d > 0)  {
            p.y--;
            d += 4 * (p.x - p.y) + 10;
        } else {
            d += 4 * p.x + 6;
        }
        _set_octant_pixels(ctx, centre, p, color);
    }
}

rend_context_t *_context_create(const uint8_t *name, uint16_t width, uint16_t height, uint8_t px_bits)
{
    uint16_t buffer_length;
    if(px_bits == 1) {
        uint16_t width_bytes = width % 8 == 0? width / 8: width / 8 + 1;
        buffer_length = width_bytes * height;
    } else {
        uint16_t px_bytes = px_bits % 8 == 0? px_bits / 8: px_bits / 8 + 1;
        buffer_length = width * height * px_bytes;
    }
    rend_context_t *ctx = malloc(sizeof(rend_context_t));
    ctx->name = name;
    ctx->dim_x = width;
    ctx->dim_y = height;
    ctx->px_bits = px_bits;
    ctx->buffer_length = buffer_length;
    ctx->buffer = malloc(buffer_length);
    memset(ctx->buffer, 0, buffer_length);
    ctx->font = malloc(sizeof(rend_font_t));
    ctx->font->id = REND_FONT_12;
    ctx->color_bg = REND_BLACK;
    ctx->color_fg = REND_WHITE;
    return ctx;
}

// TODO implement using rend_draw_point so point radius setting is observed
void _draw_circle(const rend_context_t *ctx, rend_point2d p, uint16_t radius, bool fill)
{
    _set_pixels_circle(ctx, p, radius, ctx->color_fg);
}

void _draw_point(const rend_context_t *ctx, rend_point2d p)
{
    _set_pixels_circle(ctx, p, ctx->point_radius, ctx->color_fg);
}

void _draw_line(const rend_context_t *ctx, rend_point2d p0, rend_point2d p1, bool solid)
{
    rend_point2d p;
    int8_t sx, sy;
    int32_t dx, dy, error, err2;

    dx = abs(p1.x - p0.x);
    sx = p0.x < p1.x? 1 : -1;
    dy = -1 * abs(p1.y - p0.y);
    sy = p0.y < p1.y? 1 : -1;
    error = dx + dy;
    p = p0;

    while (1)
    {
        _set_pixel(ctx, p, ctx->color_fg);
        err2 = 2 * error;

        if(err2 >= dy) {
            if(p.x == p1.x) break;
            error += dy;
            p.x += sx;
        }
        if(err2 <= dx) {
            if(p.y == p1.y) break;
            error += dx;
            p.y += sy;
        }
    }
}

void _draw_clear(const rend_context_t *ctx)
{
    if(ctx->px_bits == 1) {
        uint8_t value = ctx->color_bg? 0xFF : 0x00;
        memset(ctx->buffer, value, ctx->buffer_length);
    }
}
void _draw_text(const rend_context_t *ctx,
                    rend_point2d origin, const uint8_t *text)
{
    
}
// tail chaining function to rearrange input geometry
void _draw_rect(const rend_context_t *ctx,
                    rend_point2d p0, rend_point2d p1, bool fill)
{
        // normalize gradient of (p0 -> p1)
        if(p0.y < p1.y) {
                if(p0.x > p1.x) {
                        return _draw_rect_norm(ctx,
                                    (rend_point2d) {p1.x, p0.y},
                                    (rend_point2d) {p0.x, p1.y},
                                    fill);
                }
                return _draw_rect_norm(ctx, p1, p0, fill);
        }
        return _draw_rect_norm(ctx, p0, p1, fill);
}
void _draw_rect_norm(const rend_context_t *ctx,
                    rend_point2d p0, rend_point2d p1, bool fill)
{
        // ASSERT ( p0.y < p1.y && p0.x < p1.x )
        // assertion: the gradient from p0 -> p1 is positive
        _draw_line(ctx, p0, (rend_point2d) { p0.x, p1.y }, true);
        _draw_line(ctx, p0, (rend_point2d) { p1.x, p0.y }, true);
        _draw_line(ctx, (rend_point2d) { p0.x, p1.y }, p1, true);
        _draw_line(ctx, (rend_point2d) { p1.x, p0.y }, p1, true);
    
    
    //_rend_debug_api(draw_rect, ctx);

    //_rend_debug_api(draw_rect, ctx);
}

uint8_t *_buffer_to_string(const rend_context_t *ctx)
{
    size_t out_len;
    if(ctx->px_bits == 1) {
        out_len = (ctx->dim_x + 1) * ctx->dim_y + 1;
    }
    uint8_t *out = calloc(sizeof(uint8_t), out_len);
    memset(out, ' ', out_len);

    uint8_t *p = out;
    for(uint16_t y = 0; y < ctx->dim_y; y++) {
        for(uint16_t x = 0; x < ctx->dim_x; x++) {
            *(p++) = _get_pixel(ctx, (rend_point2d) {x,y})? '0' : '_';
        }
        *(p++) = '\n';
    }
    *p = '\0';
    return out;
}
void _buffer_print_stdout(const rend_context_t *ctx)
{
    uint8_t width = ctx->dim_x;
    uint8_t *frame = _buffer_to_string(ctx);
    uint8_t *border = calloc(sizeof(uint8_t), width + 1);
    memset(border, '#', width);
    border[width] = '\0';
    printf("%s\n%s%s\n", border, frame, border);
}