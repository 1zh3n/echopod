#pragma once

#include <stdint.h>

#include "gfx/canvas.h"
#include "ui/theme.h"

typedef struct ColorIcon {
    const uint8_t *levels;
    const Color *palette;
    uint8_t width;
    uint8_t height;
} ColorIcon;

extern const ColorIcon LITE_COLOR_ICON_ALL;
extern const ColorIcon LITE_COLOR_ICON_LIST;
extern const ColorIcon LITE_COLOR_ICON_GEAR;
extern const ColorIcon LITE_COLOR_ICON_FAVORITE;
extern const IconMask LITE_MASK_SHADOW;

void color_icon_draw_scaled(Canvas *canvas, const ColorIcon *icon, Rect area);
void color_icon_draw_glyph_scaled(Canvas *canvas, const ColorIcon *icon, Rect area, Rect source);
