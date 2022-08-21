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

#include "rend.h"

#include <string.h>

// TODO implement image transforms
// TODO implement for BPP values > 1
void _set_pixel(const rend_context_t *ctx, rend_point2d p, uint32_t color)
{
    // resolve pixel address in buffer, after 2d transforms
    if(ctx->px_bits == 1) {     // monochrome image: 8px per byte
        uint8_t width_bytes = (ctx->dim_x / 8) + ((ctx->dim_x % 8)? 1 : 0);
        uint8_t *buf_byte = &ctx->buffer[p.y * width_bytes + p.x / 8];
        if(color) {
            *buf_byte = *buf_byte | (0x80 >> p.x % 8);
            return;
        } else {
            *buf_byte = *buf_byte & ~(0x80 >> p.x % 8);
        }
    }
}

uint32_t _get_pixel(const rend_context_t *ctx, rend_point2d p)
{
    if(ctx->px_bits == 1) {     // monochrome image: 8px per byte
        uint8_t width_bytes = (ctx->dim_x / 8) + ((ctx->dim_x % 8)? 1 : 0);
        //return ctx->buffer[p.y * width_bytes + p.x / 8] & (0x80 >> p.x % 8);
        uint8_t px_block = ctx->buffer[p.y * width_bytes + p.x / 8];
        uint32_t out = px_block & (0x80 >> p.x % 8);
        return out;
    }
    return 0;
}

// TODO add optimized version to fill all pixels along horizontal spans using memset
void _set_octant_pixels(const rend_context_t *ctx, rend_point2d centre, rend_point2d p, uint32_t color)
{
    int16_t dx, dy;
    dx = p.x - centre.x;
    dy = p.y - centre.y;

    // iterate through octants in clockwise order
    _set_pixel(ctx, p, color);
    _set_pixel(ctx, (rend_point2d) { centre.x + dy, centre.y + dx }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x + dy, centre.y - dx }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x + dx, centre.y - dy }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x - dx, centre.y - dy }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x - dy, centre.y - dx }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x - dy, centre.y + dx }, color);
    _set_pixel(ctx, (rend_point2d) { centre.x - dx, centre.y + dy }, color);
}

void _set_pixels_circle(const rend_context_t *ctx, rend_point2d centre, uint8_t radius, uint32_t color)
{
    int16_t d, d_e, d_se;
    rend_point2d p = {centre.x, centre.y + radius};
    d = 1 - radius;
    d_e = 3;
    d_se = -2 * radius + 5;

    while(p.y > p.x) {
        _set_octant_pixels(ctx, centre, p, color);
        if(d < 0) {
            d += 2 * p.x + 3;
        } else {
            d += 2 * (p.x - p.y) + 5;
            p.y--;
        }
        p.x++;
    }
}

rend_context_t *rend_context_create(uint16_t width, uint16_t height, uint8_t px_bits)
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
    ctx->dim_x = width;
    ctx->dim_y = height;
    ctx->px_bits = px_bits;
    ctx->buffer_length = buffer_length;
    ctx->buffer = malloc(buffer_length);
    ctx->font = malloc(sizeof(rend_font_t));
    ctx->font->id = REND_FONT_12;
    ctx->color_bg = REND_BLACK;
    ctx->color_fg = REND_WHITE;
    return ctx;
}

// TODO implement using rend_draw_point so point radius setting is observed
void rend_draw_circle(const rend_context_t *ctx, rend_point2d centre, uint16_t radius, bool fill)
{
    _set_pixels_circle(ctx, centre, radius, ctx->color_fg);
}

void rend_draw_point(const rend_context_t *ctx, rend_point2d p)
{
    _set_pixels_circle(ctx, p, ctx->point_radius, ctx->color_fg);
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

        if((2 * error) >= dy) {
            if(p.x == p1.x) break;
            error += dy;
            p.x += sx;
        }
        if((2 * error) <= dx) {
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

#ifndef PICO_RP2040
uint8_t *rend_print_buffer(const rend_context_t *ctx)
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
#endif
