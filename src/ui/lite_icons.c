#include "ui/lite_icons.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef ECHO_LITE
#include "hal/system.h"

enum {
    ICON_BUFFER_BYTES = 2048,
    ICON_CACHE_SLOTS = 4
};

static uint8_t icon_buffers[ICON_CACHE_SLOTS][ICON_BUFFER_BYTES];
static uint32_t icon_offsets[ICON_CACHE_SLOTS];
static bool icon_slot_valid[ICON_CACHE_SLOTS];
static uint8_t icon_next_slot;

const uint8_t *lite_icon_levels(const uint8_t *encoded, uint32_t bytes)
{
    uintptr_t address = (uintptr_t)encoded;
    if (address < LITE_ICON_POINTER_BASE || address >= LITE_ICON_POINTER_LIMIT ||
        bytes > ICON_BUFFER_BYTES) {
        return NULL;
    }
    uint32_t offset = (uint32_t)(address - LITE_ICON_POINTER_BASE);
    for (uint8_t slot = 0; slot < ICON_CACHE_SLOTS; slot++) {
        if (icon_slot_valid[slot] && icon_offsets[slot] == offset) {
            return icon_buffers[slot];
        }
    }
    uint8_t slot = icon_next_slot;
    icon_next_slot = (uint8_t)((slot + 1) % ICON_CACHE_SLOTS);
    if (!hal_system_lite_icons_read(offset, icon_buffers[slot], bytes)) {
        icon_slot_valid[slot] = false;
        return NULL;
    }
    icon_offsets[slot] = offset;
    icon_slot_valid[slot] = true;
    return icon_buffers[slot];
}
#endif

static uint8_t level_at(const uint8_t *levels, int width, int x, int y)
{
    uint8_t pair = levels[y * ((width + 1) / 2) + x / 2];
    return (x & 1) != 0 ? pair & 0x0F : pair >> 4;
}

void lite_icon_draw_scaled(Canvas *canvas, const IconMask *mask, Rect area, Color color)
{
    if (mask == NULL || area.width <= 0 || area.height <= 0) {
        return;
    }
    const uint8_t *levels = mask->levels;
#ifdef ECHO_LITE
    levels = lite_icon_levels(levels, (uint32_t)mask->width * mask->height / 2);
#endif
    if (levels == NULL) {
        return;
    }
    for (int y = 0; y < area.height; y++) {
        int source_y = y * mask->height / area.height;
        for (int x = 0; x < area.width; x++) {
            int source_x = x * mask->width / area.width;
            uint8_t level = level_at(levels, mask->width, source_x, source_y);
            if (level != 0) {
                canvas_blend_pixel(canvas, (Point){area.x + x, area.y + y}, color, level);
            }
        }
    }
}
