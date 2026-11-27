#include "lite_animation.h"

#ifdef ECHO_LITE

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "hal/clock.h"
#include "hal/system.h"
#include "ui/lite_color_icons.h"

enum {
    HEADER_BYTES = 40,
    BOOT_HEADER_BYTES = 48,
    BOOT_SHADOW = 5,
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

bool lite_boot_draw(Canvas *canvas, Rect screen, Color shadow, bool portrait)
{
    if (canvas == NULL || !load()) {
        return false;
    }
    uint32_t boot_offset = HEADER_BYTES + (uint32_t)animation.frame_count * animation.frame_bytes;
    uint32_t size = hal_system_lite_animation_size();
    if (size < boot_offset + BOOT_HEADER_BYTES) {
        return false;
    }
    uint8_t header[BOOT_HEADER_BYTES];
    if (!hal_system_lite_animation_read(boot_offset, header, sizeof header)) {
        return false;
    }
    if (header[0] != 'L' || header[1] != 'B' || header[2] != 'O' || header[3] != 'O' ||
        header[4] != 1) {
        return false;
    }
    uint32_t header_offset = boot_offset;
    if (portrait) {
        int landscape_width = header[6] | (header[7] << 8);
        int landscape_height = header[8] | (header[9] << 8);
        if (landscape_width <= 0 || landscape_height <= 0 || (landscape_width & 1) != 0) {
            return false;
        }
        header_offset += BOOT_HEADER_BYTES +
                         (uint32_t)landscape_width * (uint32_t)landscape_height / 2;
        if (size < header_offset + BOOT_HEADER_BYTES ||
            !hal_system_lite_animation_read(header_offset, header, sizeof header)) {
            return false;
        }
        if (header[0] != 'L' || header[1] != 'B' || header[2] != 'O' || header[3] != '2' ||
            header[4] != 1) {
            return false;
        }
    }
    int width = header[6] | (header[7] << 8);
    int height = header[8] | (header[9] << 8);
    if (width <= 0 || height <= 0 || width > screen.width || height > screen.height ||
        (width & 1) != 0) {
        return false;
    }
    Rect image = {screen.x + (screen.width - width) / 2,
                  screen.y + (screen.height - height) / 2, width, height};
    canvas_fill_rect(canvas, (Rect){image.x + BOOT_SHADOW, image.y + BOOT_SHADOW, image.width,
                                    image.height},
                     shadow);
    uint16_t palette[PALETTE_COLORS];
    for (int index = 0; index < PALETTE_COLORS; index++) {
        palette[index] = (uint16_t)(header[12 + index * 2] | (header[13 + index * 2] << 8));
    }
    uint8_t row[160];
    int row_bytes = width / 2;
    if (row_bytes > (int)sizeof row) {
        return false;
    }
    uint32_t data_offset = header_offset + BOOT_HEADER_BYTES;
    int clip_top = canvas->clip.y;
    int clip_bottom = clip_top + canvas->clip.height;
    for (int y = 0; y < height; y++) {
        int screen_y = image.y + y;
        if (screen_y < clip_top || screen_y >= clip_bottom) {
            continue;
        }
        if (!hal_system_lite_animation_read(data_offset + (uint32_t)y * (uint32_t)row_bytes, row,
                                            (uint32_t)row_bytes)) {
            return false;
        }
        int x = 0;
        while (x < width) {
            uint8_t byte = row[x >> 1];
            uint8_t index = (x & 1) != 0 ? (byte & 0x0F) : (byte >> 4);
            int run = 1;
            while (x + run < width) {
                int next_x = x + run;
                uint8_t next_byte = row[next_x >> 1];
                uint8_t next = (next_x & 1) != 0 ? (next_byte & 0x0F) : (next_byte >> 4);
                if (next != index) {
                    break;
                }
                run++;
            }
            canvas_fill_rect(canvas, (Rect){image.x + x, image.y + y, run, 1},
                             palette[index & (PALETTE_COLORS - 1)]);
            x += run;
        }
    }
    return true;
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

static void draw_box(Canvas *canvas, Rect area, int width, Color ink)
{
    canvas_fill_rect(canvas, (Rect){area.x, area.y, area.width, width}, ink);
    canvas_fill_rect(canvas, (Rect){area.x, area.y + area.height - width, area.width, width},
                     ink);
    canvas_fill_rect(canvas, (Rect){area.x, area.y, width, area.height}, ink);
    canvas_fill_rect(canvas, (Rect){area.x + area.width - width, area.y, width, area.height},
                     ink);
}

void lite_animation_draw_card(Canvas *canvas, Rect card, Color shadow, Color card_border,
                              int card_border_width, Color frame_ink, Color panel,
                              uint32_t now_ms, bool animate)
{
    Rect frame = {card.x + (card.width - FRAME_WIDTH) / 2,
                  card.y + (card.height - FRAME_HEIGHT) / 2, FRAME_WIDTH, FRAME_HEIGHT};
    canvas_fill_rect(canvas,
                     (Rect){card.x + CARD_SHADOW, card.y + CARD_SHADOW, card.width, card.height},
                     shadow);
    canvas_fill_rect(canvas, card, panel);
    draw_box(canvas, card, card_border_width, card_border);
    canvas_fill_rect(
        canvas, (Rect){frame.x + FRAME_SHADOW, frame.y + FRAME_SHADOW, frame.width, frame.height},
        frame_ink);
    canvas_fill_rect(canvas, frame, panel);
    draw_box(canvas, frame, CARD_BORDER, frame_ink);
    if (load()) {
        if (animate) {
            advance(now_ms);
        }
        draw_frame(canvas,
                   (Rect){frame.x + FRAME_BORDER, frame.y + FRAME_BORDER, animation.width,
                          animation.height});
    } else {
        color_icon_draw_scaled(canvas, &LITE_COLOR_ICON_ALL,
                               (Rect){frame.x + (FRAME_WIDTH - FALLBACK_ICON) / 2,
                                      frame.y + (FRAME_HEIGHT - FALLBACK_ICON) / 2, FALLBACK_ICON,
                                      FALLBACK_ICON});
    }
}

#else
typedef int LiteAnimationTranslationUnit;

#endif
