/*
 *  rend.c
 *  routines for managing and drawing to image buffers
 * 
 *  TODO: implement optimized methods for filling consecutive
 *          pixels in memory simultaneously
 *  
*/

#include "rend.h"

#include <string.h>

// TODO implement image transforms
// TODO implement for BPP values > 1
void _set_pixel(const rend_context_t *ctx, rend_point2d p, uint32_t color)
{
    // resolve pixel address in buffer, after 2d transforms
    if(ctx->px_bits == 1) {     // monochrome image: 8px per byte
        uint8_t width_bytes = (ctx->dim_x / 8) + (ctx->dim_x % 8)? 1 : 0;
        uint8_t *buf_byte = &ctx->buffer[p.y * width_bytes + p.x / 8];
        if(color) {
            *buf_byte = *buf_byte & ~(0x80 >> p.x % 8);
        } else {
            *buf_byte = *buf_byte | (0x80 >> p.x % 8);
        }
    }
}

void _set_octant_pixels(const rend_context_t *ctx, rend_point2d centre, rend_point2d p)
{
    
}

void _set_pixels_circle(const rend_context_t *ctx, rend_point2d centre, uint8_t radius)
{
    int16_t d, d_e, d_se;
    rend_point2d p = {centre.x, centre.y + radius};
    d = 1 - radius;
    d_e = 3;
    d_se = -2 * radius + 5;

}

rend_context_t *rend_context_create(uint16_t width, uint16_t height, uint8_t px_bits)
{
    uint16_t buffer_length;
    if(px_bits == 1) {
        uint16_t width_bytes = width % 8 == 0? width / 8: width / 8 + 1;
        uint16_t height_bytes = height % 8 == 0? height / 8: height / 8 + 1;
        buffer_length = width_bytes * height_bytes;
    } else {
        uint16_t px_bytes = px_bits % 8 == 0? px_bits / 8: px_bits / 8 + 1;
        buffer_length = width * height * px_bytes;
    }
    rend_context_t *ctx = malloc(sizeof(rend_context_t));
    ctx->px_bits = px_bits;
    ctx->buffer_length = buffer_length;
    ctx->buffer = malloc(buffer_length);
    ctx->font = malloc(sizeof(rend_font_t));
    ctx->font->id = REND_FONT_12;
    ctx->color_bg = REND_BLACK;
    ctx->color_fg = REND_WHITE;
    return ctx;
}

void rend_draw_circle(const rend_context_t *ctx, rend_point2d centre, uint16_t radius, bool fill)
{

}

void rend_draw_point(const rend_context_t *ctx, rend_point2d p)
{
    _set_pixel(ctx, p, ctx->color_fg);
    for(int i = 0; i < ctx->point_size; i++) {
        for(int j = 0; j < i; j++) {

        }
    }
}

void rend_draw_line(const rend_context_t *ctx, rend_point2d p0, rend_point2d p1, bool solid)
{
    rend_point2d p;
    int8_t sx, sy;
    int32_t x, y, dx, dy, error;

    dx = abs(p1.x - p0.x);
    sx = p0.x < p1.x? 1 : -1;
    dy = -1 * abs(p1.y - p0.y);
    sy = p0.y < p1.y? 1 : -1;
    error = dx + dy;
    p = p0;

    while (1)
    {
        rend_draw_point(ctx, p);
        if(p.x == p1.x && p.y == p1.y) break;

        if((2 * error) > dy) {
            if(p.x == p1.x) break;
            error += dy;
            p.x += sx;
        }
        if((2 * error) > dx) {
            if(p.y == p1.y) break;
            error += dx;
            p.y += sy;
        }
    }
}

void rend_draw_clear(const rend_context_t *ctx)
{
    if(ctx->px_bits == 1) {
        uint8_t value = ctx->color_bg? 0xFF : 0x00;
        memset(ctx->buffer, value, ctx->buffer_length);
    }
}
void rend_draw_text(const rend_context_t *ctx,
                    rend_point2d origin, const uint8_t *text)
{
    
}
void rend_draw_rect(const rend_context_t *ctx,
                    rend_point2d origin, rend_point2d extent, bool fill)
{
    
}
