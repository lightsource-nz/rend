#ifndef _REND_H
#define _REND_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define REND_BLACK      (uint16_t)0x0000
#define REND_WHITE      (uint16_t)0xFFFF

#define REND_FONT_8   0
#define REND_FONT_12  1
#define REND_FONT_16  2
#define REND_FONT_20  3
#define REND_FONT_24  4

#define REND_POINT_1    1
#define REND_POINT_2    2
#define REND_POINT_3    3
#define REND_POINT_4    4
#define REND_POINT_5    5
#define REND_POINT_6    6
#define REND_POINT_7    7
#define REND_POINT_8    8

typedef struct rend_font {
    uint8_t id;
} rend_font_t;

typedef struct rend_context {
    uint8_t *buffer;
    size_t buffer_length;
    uint16_t dim_x;
    uint16_t dim_y;
    uint8_t px_bits;
    uint8_t px_bytes;
    uint8_t point_radius;
    uint16_t color_fg;
    uint16_t color_bg;
    rend_font_t *font;
} rend_context_t;

typedef struct rend_point2d {
    uint16_t x;
    uint16_t y;
} rend_point2d;

rend_context_t *rend_context_create(uint16_t width, uint16_t height, uint8_t px_bits);

void rend_draw_circle(const rend_context_t *ctx, rend_point2d centre, uint16_t radius, bool fill);
void rend_draw_point(const rend_context_t *img, rend_point2d p);
void rend_draw_line(const rend_context_t *ctx, rend_point2d p0, rend_point2d p1, bool solid);
void rend_draw_clear(const rend_context_t *ctx);
void rend_draw_text(const rend_context_t *ctx,
                    rend_point2d p, const uint8_t *text);
void rend_draw_rect(const rend_context_t *ctx,
                    rend_point2d p0, rend_point2d p1, bool fill);

#ifndef PICO_RP2040
uint8_t *rend_print_buffer(const rend_context_t *ctx);
#endif

#endif