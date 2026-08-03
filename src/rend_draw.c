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
 *  --> TODO: implement support for grayscale and colour pixel formats
 *
 *  rotation and mirroring are implemented as composable affine transforms
 *  (_recompute_transform()/_apply_transform() / rend_context_set_rotation()/
 *  rend_context_set_flip())
 */

#include <rend.h>
#include "rend_internal.h"

#include <stdio.h>
#include <string.h>

// TODO implement for BPP values > 1
// composes two affine transforms represented as their equivalent 3x3 homogeneous
// matrices (bottom row [0 0 1], not stored): result = second . first, i.e. a point is
// mapped by 'first' and then by 'second'
static rend_transform_t _compose_transform(rend_transform_t first, rend_transform_t second)
{
    return (rend_transform_t) {
        .a  = second.a * first.a + second.b * first.c,
        .b  = second.a * first.b + second.b * first.d,
        .tx = second.a * first.tx + second.b * first.ty + second.tx,
        .c  = second.c * first.a + second.d * first.c,
        .d  = second.c * first.b + second.d * first.d,
        .ty = second.c * first.tx + second.d * first.ty + second.ty
    };
}
// recomputes ctx->transform from ctx->rotation/ctx->flip/phys_dim_x/phys_dim_y. called
// whenever any of those change (_context_set_rotation()/_context_set_flip()) rather
// than re-deriving the matrix on every pixel
static void _recompute_transform(rend_context_t *ctx)
{
    // logical dim_x/dim_y depend only on rotation -- flip mirrors within whatever
    // logical space rotation already established, it doesn't change the dimensions
    if(ctx->rotation == REND_ROTATE_90 || ctx->rotation == REND_ROTATE_270) {
        ctx->dim_x = ctx->phys_dim_y;
        ctx->dim_y = ctx->phys_dim_x;
    } else {
        ctx->dim_x = ctx->phys_dim_x;
        ctx->dim_y = ctx->phys_dim_y;
    }

    rend_transform_t flip;
    switch(ctx->flip) {
    case REND_FLIP_HORIZONTAL:
        flip = (rend_transform_t) { .a = -1, .b = 0, .tx = ctx->dim_x - 1, .c = 0, .d = 1,  .ty = 0 };
        break;
    case REND_FLIP_VERTICAL:
        flip = (rend_transform_t) { .a = 1,  .b = 0, .tx = 0, .c = 0, .d = -1, .ty = ctx->dim_y - 1 };
        break;
    case REND_FLIP_BOTH:
        flip = (rend_transform_t) { .a = -1, .b = 0, .tx = ctx->dim_x - 1, .c = 0, .d = -1, .ty = ctx->dim_y - 1 };
        break;
    case REND_FLIP_NONE:
    default:
        flip = (rend_transform_t) { .a = 1, .b = 0, .tx = 0, .c = 0, .d = 1, .ty = 0 };
        break;
    }

    rend_transform_t rotate;
    switch(ctx->rotation) {
    case REND_ROTATE_90:
        // logical top edge -> physical right edge
        rotate = (rend_transform_t) {
            .a = 0, .b = -1, .tx = ctx->phys_dim_x - 1,
            .c = 1, .d = 0,  .ty = 0
        };
        break;
    case REND_ROTATE_180:
        rotate = (rend_transform_t) {
            .a = -1, .b = 0,  .tx = ctx->phys_dim_x - 1,
            .c = 0,  .d = -1, .ty = ctx->phys_dim_y - 1
        };
        break;
    case REND_ROTATE_270:
        // logical top edge -> physical left edge
        rotate = (rend_transform_t) {
            .a = 0,  .b = 1, .tx = 0,
            .c = -1, .d = 0, .ty = ctx->phys_dim_y - 1
        };
        break;
    case REND_ROTATE_0:
    default:
        rotate = (rend_transform_t) {
            .a = 1, .b = 0, .tx = 0,
            .c = 0, .d = 1, .ty = 0
        };
        break;
    }

    // flip is applied first (in logical space), then rotate maps the (possibly
    // flipped) logical point onto the physical buffer
    ctx->transform = _compose_transform(flip, rotate);
}
// maps a LOGICAL (dim_x, dim_y)-bounded coordinate onto the PHYSICAL
// (phys_dim_x, phys_dim_y)-bounded buffer via ctx->transform (see rend.h for the
// matrix layout). REND_ROTATE_0's identity matrix makes this a no-op multiply
static rend_point2d _apply_transform(const rend_context_t *ctx, rend_point2d p)
{
    const rend_transform_t *m = &ctx->transform;
    int32_t x = m->a * (int32_t)p.x + m->b * (int32_t)p.y + m->tx;
    int32_t y = m->c * (int32_t)p.x + m->d * (int32_t)p.y + m->ty;
    return (rend_point2d) { (uint16_t)x, (uint16_t)y };
}

void _set_pixel(const rend_context_t *ctx, rend_point2d p, uint32_t color)
{
#ifdef REND_DEBUG_PIXEL_TRACE
    trace_log_f("(%d,%d) = %d", p.x, p.y, color);
#endif
    rend_point2d phys = _apply_transform(ctx, p);
    // resolve pixel address in buffer, after 2d transforms
    if(ctx->px_bits == 1) {     // monochrome image: 8px per byte
        uint8_t width_bytes = (ctx->phys_dim_x / 8) + ((ctx->phys_dim_x % 8)? 1 : 0);
        uint8_t *buf_byte = &ctx->buffer[phys.y * width_bytes + phys.x / 8];
        if(color) {
            *buf_byte = *buf_byte | (1 << phys.x % 8);
        } else {
            *buf_byte = *buf_byte & ~(1 << phys.x % 8);
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
    rend_point2d phys = _apply_transform(ctx, p);
    if(ctx->px_bits == 1) {     // monochrome image: 8px per byte
        uint8_t width_bytes = (ctx->phys_dim_x / 8) + ((ctx->phys_dim_x % 8)? 1 : 0);
        uint8_t px_block = ctx->buffer[phys.y * width_bytes + phys.x / 8];
        uint32_t out = px_block & (1 << phys.x % 8);
        return out;
    }
    return 0;
}

// rend_point2d's fields are uint16_t, so an expression like "centre.x - p.y" that goes
// negative (e.g. drawing a circle/point near the top-left of the canvas) silently
// wraps to a huge unsigned value once stored back into a rend_point2d -- _set_pixel()
// would then index far outside the buffer. this clips any such point instead of
// wrapping, computing each candidate coordinate as a signed value first
static void _set_pixel_clipped(const rend_context_t *ctx, int32_t x, int32_t y, uint32_t color)
{
    if(x < 0 || y < 0 || x >= ctx->dim_x || y >= ctx->dim_y)
        return;
    _set_pixel(ctx, (rend_point2d) { (uint16_t)x, (uint16_t)y }, color);
}
// TODO add optimized version to fill all pixels along horizontal spans using memset
void _set_octant_pixels(const rend_context_t *ctx, rend_point2d centre, rend_point2d p, uint32_t color)
{
    int32_t cx = centre.x, cy = centre.y, px = p.x, py = p.y;
    // iterate through octants in clockwise order
    _set_pixel_clipped(ctx, cx + px, cy + py, color);
    _set_pixel_clipped(ctx, cx + py, cy + px, color);
    _set_pixel_clipped(ctx, cx + py, cy - px, color);
    _set_pixel_clipped(ctx, cx + px, cy - py, color);
    _set_pixel_clipped(ctx, cx - px, cy - py, color);
    _set_pixel_clipped(ctx, cx - py, cy - px, color);
    _set_pixel_clipped(ctx, cx - py, cy + px, color);
    _set_pixel_clipped(ctx, cx - px, cy + py, color);
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
    ctx->phys_dim_x = width;
    ctx->phys_dim_y = height;
    ctx->rotation = REND_ROTATE_0;
    ctx->flip = REND_FLIP_NONE;
    _recompute_transform(ctx);
    ctx->px_bits = px_bits;
    ctx->buffer_length = buffer_length;
    ctx->buffer = malloc(buffer_length);
    memset(ctx->buffer, 0, buffer_length);
    ctx->buffer_back = NULL;
    ctx->font = NULL;
    ctx->color_bg = REND_BLACK;
    ctx->color_fg = REND_WHITE;
    return ctx;
}

void _context_set_font(rend_context_t *ctx, const rend_font_t *font)
{
    ctx->font = font;
}

void _context_enable_double_buffer(rend_context_t *ctx)
{
    if(ctx->buffer_back)
        return;
    ctx->buffer_back = malloc(ctx->buffer_length);
    memset(ctx->buffer_back, 0, ctx->buffer_length);
}

bool _context_swap_buffers(rend_context_t *ctx)
{
    if(!ctx->buffer_back)
        return false;
    uint8_t *front = ctx->buffer;
    ctx->buffer = ctx->buffer_back;
    ctx->buffer_back = front;
    return true;
}

void _context_set_rotation(rend_context_t *ctx, uint8_t rotation)
{
    ctx->rotation = rotation;
    // dim_x/dim_y are (re)derived inside _recompute_transform(), since flip also
    // depends on them and both need to stay in sync with whichever changed last
    _recompute_transform(ctx);
}

void _context_set_flip(rend_context_t *ctx, uint8_t flip)
{
    ctx->flip = flip;
    _recompute_transform(ctx);
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
    if(!ctx->font) return;
    const rend_font_t *font = ctx->font;
    uint8_t pitch = (font->char_width + 7) / 8;
    rend_point2d pen = origin;
    for(const uint8_t *c = text; *c; c++) {
        if(*c < REND_FONT_GLYPH_TABLE_SIZE) {
            const uint8_t *glyph = font->glyphs[*c];
            if(glyph) {
                for(uint8_t y = 0; y < font->char_height; y++) {
                    for(uint8_t x = 0; x < font->char_width; x++) {
                        uint8_t byte = glyph[y * pitch + x / 8];
                        if((byte >> (7 - (x % 8))) & 1) {
                            _set_pixel(ctx, (rend_point2d) { pen.x + x, pen.y + y }, ctx->color_fg);
                        }
                    }
                }
            }
        }
        pen.x += font->char_width;
    }
}
// tail chaining function to rearrange input geometry -- callers may pass either diagonal's
// two corners in any order, so normalize independently per axis into (top-left, bottom-right)
// before handing off to _draw_rect_norm(), which assumes that canonical form
void _draw_rect(const rend_context_t *ctx,
                    rend_point2d p0, rend_point2d p1, bool fill)
{
        rend_point2d top_left     = { p0.x < p1.x ? p0.x : p1.x, p0.y < p1.y ? p0.y : p1.y };
        rend_point2d bottom_right = { p0.x > p1.x ? p0.x : p1.x, p0.y > p1.y ? p0.y : p1.y };
        return _draw_rect_norm(ctx, top_left, bottom_right, fill);
}
void _draw_rect_norm(const rend_context_t *ctx,
                    rend_point2d p0, rend_point2d p1, bool fill)
{
        // ASSERT ( p0.y < p1.y && p0.x < p1.x )
        // assertion: the gradient from p0 -> p1 is positive
        if(fill) {
                for(uint16_t y = p0.y; y <= p1.y; y++) {
                        _draw_line(ctx, (rend_point2d) { p0.x, y }, (rend_point2d) { p1.x, y }, true);
                }
                return;
        }
        _draw_line(ctx, p0, (rend_point2d) { p0.x, p1.y }, true);
        _draw_line(ctx, p0, (rend_point2d) { p1.x, p0.y }, true);
        _draw_line(ctx, (rend_point2d) { p0.x, p1.y }, p1, true);
        _draw_line(ctx, (rend_point2d) { p1.x, p0.y }, p1, true);
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