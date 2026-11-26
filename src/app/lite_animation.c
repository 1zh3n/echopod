#include "lite_animation.h"

#ifdef ECHO_LITE

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "hal/clock.h"
#include "hal/system.h"
#include "ui/lite_icons.h"

enum {
    HEADER_BYTES = 40,
    PALETTE_BYTES = 32,
    FRAME_MS = 100,
    PALETTE_COLORS = 16,
    CARD_WIDTH = 200,
    CARD_HEIGHT = 112,
    FRAME_WIDTH = 100,
    FRAME_HEIGHT = 68,
    FRAME_BORDER = 2,
    CARD_BORDER = 2,
    CARD_SHADOW = 5,
    FRAME_SHADOW = 3,
    FALLBACK_ICON = 40
};

typedef struct LiteAnimation {
    bool ready;
    bool attempted;
    uint8_t frame_count;
    uint8_t width;
    uint8_t height;
    uint16_t frame_bytes;
    uint16_t palette[PALETTE_COLORS];
    uint8_t *buffers[2];
    uint8_t active;
    uint8_t frame;
    uint32_t next_ms;
} LiteAnimation;

static LiteAnimation animation;

void lite_animation_reset(void)
{
    animation = (LiteAnimation){0};
}

bool lite_animation_ready(void)
{
    return animation.ready;
}

uint8_t lite_animation_frames(void)
{
    return animation.frame_count;
}

static bool load(void)
{
    if (animation.ready) {
        return true;
    }
    if (animation.attempted) {
        return false;
    }
    animation.attempted = true;
    uint32_t arena_size = 0;
    uint8_t *arena = hal_system_lite_animation(&arena_size);
    uint32_t size = hal_system_lite_animation_size();
    if (arena == NULL || size < HEADER_BYTES) {
        return false;
    }
    uint8_t header[HEADER_BYTES];
    if (!hal_system_lite_animation_read(0, header, sizeof header)) {
        return false;
    }
    if (header[0] != 'L' || header[1] != 'A' || header[2] != 'N' || header[3] != 'I' ||
        header[4] != 1) {
        return false;
    }
    uint8_t count = header[5];
    uint8_t width = header[6];
    uint8_t height = header[7];
    if (count == 0 || width == 0 || height == 0 || (width & 1) != 0) {
        return false;
    }
    uint32_t frame_bytes = (uint32_t)width * height / 2;
    uint32_t needed = HEADER_BYTES + (uint32_t)count * frame_bytes;
    if (needed > size || frame_bytes * 2 > arena_size) {
        return false;
    }
    memcpy(animation.palette, header + 8, PALETTE_BYTES);
    animation.buffers[0] = arena;
    animation.buffers[1] = arena + frame_bytes;
    animation.frame_count = count;
    animation.width = width;
    animation.height = height;
    animation.frame_bytes = (uint16_t)frame_bytes;
    animation.active = 0;
    animation.frame = 0;
    animation.next_ms = hal_clock_milliseconds() + FRAME_MS;
    if (!hal_system_lite_animation_read(HEADER_BYTES, animation.buffers[0], frame_bytes)) {
        return false;
    }
    animation.ready = true;
    return true;
}

bool lite_animation_load(void)
{
    return load();
}

static void advance(uint32_t now_ms)
{
    for (int guard = 0; guard < 2 && (int32_t)(now_ms - animation.next_ms) >= 0; guard++) {
        uint8_t next_frame = (uint8_t)((animation.frame + 1) % animation.frame_count);
        uint8_t next_buffer = (uint8_t)(animation.active ^ 1);
        uint32_t offset = HEADER_BYTES + (uint32_t)next_frame * animation.frame_bytes;
        if (!hal_system_lite_animation_read(offset, animation.buffers[next_buffer],
                                            animation.frame_bytes)) {
            animation.next_ms = now_ms + FRAME_MS;
            return;
        }
        animation.active = next_buffer;
        animation.frame = next_frame;
        animation.next_ms += FRAME_MS;
    }
    if ((int32_t)(now_ms - animation.next_ms) >= 0) {
        animation.next_ms = now_ms + FRAME_MS;
    }
}

static Color pixel(const uint8_t *frame, int x, int y)
{
    const uint8_t *row = frame + (uint32_t)y * (animation.width / 2);
    uint8_t byte = row[x >> 1];
    uint8_t index = (x & 1) != 0 ? (uint8_t)(byte & 0x0F) : (uint8_t)(byte >> 4);
    return animation.palette[index & (PALETTE_COLORS - 1)];
}

static void draw_frame(Canvas *canvas, Rect area)
{
    if (area.width != animation.width || area.height != animation.height) {
        return;
    }
    Rect clip = canvas->clip;
    int left = area.x > clip.x ? area.x : clip.x;
    int top = area.y > clip.y ? area.y : clip.y;
    int right = area.x + area.width;
    int bottom = area.y + area.height;
    if (right > clip.x + clip.width) {
        right = clip.x + clip.width;
    }
    if (bottom > clip.y + clip.height) {
        bottom = clip.y + clip.height;
    }
    if (left >= right || top >= bottom) {
        return;
    }
    const uint8_t *frame = animation.buffers[animation.active];
    for (int y = top; y < bottom; y++) {
        int x = left;
        while (x < right) {
            Color color = pixel(frame, x - area.x, y - area.y);
            int run = 1;
            while (x + run < right && pixel(frame, x + run - area.x, y - area.y) == color) {
                run++;
            }
            canvas_fill_rect(canvas, (Rect){x, y, run, 1}, color);
            x += run;
        }
    }
}

static void draw_box(Canvas *canvas, Rect area, Color ink)
{
    canvas_fill_rect(canvas, (Rect){area.x, area.y, area.width, CARD_BORDER}, ink);
    canvas_fill_rect(canvas, (Rect){area.x, area.y + area.height - CARD_BORDER, area.width,
                                    CARD_BORDER},
                     ink);
    canvas_fill_rect(canvas, (Rect){area.x, area.y, CARD_BORDER, area.height}, ink);
    canvas_fill_rect(canvas, (Rect){area.x + area.width - CARD_BORDER, area.y, CARD_BORDER,
                                    area.height},
                     ink);
}

void lite_animation_draw_card(Canvas *canvas, Rect card, Color shadow, Color ink, Color panel,
                              uint32_t now_ms, bool animate)
{
    Rect frame = {card.x + (card.width - FRAME_WIDTH) / 2,
                  card.y + (card.height - FRAME_HEIGHT) / 2, FRAME_WIDTH, FRAME_HEIGHT};
    canvas_fill_rect(canvas,
                     (Rect){card.x + CARD_SHADOW, card.y + CARD_SHADOW, card.width, card.height},
                     shadow);
    canvas_fill_rect(canvas, card, panel);
    draw_box(canvas, card, ink);
    canvas_fill_rect(
        canvas, (Rect){frame.x + FRAME_SHADOW, frame.y + FRAME_SHADOW, frame.width, frame.height},
        ink);
    canvas_fill_rect(canvas, frame, panel);
    draw_box(canvas, frame, ink);
    if (load()) {
        if (animate) {
            advance(now_ms);
        }
        draw_frame(canvas,
                   (Rect){frame.x + FRAME_BORDER, frame.y + FRAME_BORDER, animation.width,
                          animation.height});
    } else {
        lite_icon_draw_scaled(canvas, &LITE_ICON_MUSIC_MENU,
                              (Rect){frame.x + (FRAME_WIDTH - FALLBACK_ICON) / 2,
                                     frame.y + (FRAME_HEIGHT - FALLBACK_ICON) / 2, FALLBACK_ICON,
                                     FALLBACK_ICON},
                              ink);
    }
}

#else
typedef int LiteAnimationTranslationUnit;

#endif
