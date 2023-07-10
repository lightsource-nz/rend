#ifndef _REND_INTERNAL_H
#define _REND_INTERNAL_H

#ifndef _REND_H
#error "This file must be included after rend.h"
#endif

#include <stdint.h>
#include <stdbool.h>

#ifdef REND_API_TRACE
#   define trace_log(...) printf("TRACE: " __func__ __VA_ARGS__ "\n");
#else
#   define trace_log(...)
#endif

rend_context_t *_context_create(uint16_t width, uint16_t height, uint8_t px_bits);

void _draw_circle(const rend_context_t *ctx, rend_point2d centre, uint16_t radius, bool fill);
void _draw_point(const rend_context_t *img, rend_point2d p);
void _draw_line(const rend_context_t *ctx, rend_point2d p0, rend_point2d p1, bool solid);
void _draw_clear(const rend_context_t *ctx);
void _draw_text(const rend_context_t *ctx,
                    rend_point2d p, const uint8_t *text);
void _draw_rect(const rend_context_t *ctx,
                    rend_point2d p0, rend_point2d p1, bool fill);

uint8_t *_buffer_to_string(const rend_context_t *ctx);
void _buffer_print_stdout(const rend_context_t *ctx);

#endif