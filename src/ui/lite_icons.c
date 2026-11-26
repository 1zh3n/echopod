#include "ui/lite_icons.h"

#include <stddef.h>
#include <stdint.h>


static uint8_t level_at(const IconMask *mask, int x, int y)
{
    uint8_t pair = mask->levels[y * ((mask->width + 1) / 2) + x / 2];
    return (x & 1) != 0 ? pair & 0x0F : pair >> 4;
}

void lite_icon_draw_scaled(Canvas *canvas, const IconMask *mask, Rect area, Color color)
{
    if (mask == NULL || area.width <= 0 || area.height <= 0) {
        return;
    }
    for (int y = 0; y < area.height; y++) {
        int source_y = y * mask->height / area.height;
        for (int x = 0; x < area.width; x++) {
            int source_x = x * mask->width / area.width;
            uint8_t level = level_at(mask, source_x, source_y);
            if (level != 0) {
                canvas_blend_pixel(canvas, (Point){area.x + x, area.y + y}, color, level);
            }
        }
    }
}
