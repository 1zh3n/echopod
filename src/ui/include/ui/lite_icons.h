#pragma once

#include "gfx/canvas.h"
#include "ui/theme.h"

extern const IconMask LITE_ICON_MUSIC_MENU;
extern const IconMask LITE_ICON_MUSIC_LIST;
extern const IconMask LITE_ICON_SETTING_TWO;
extern const IconMask LITE_ICON_SHUFFLE;
extern const IconMask LITE_ICON_LOOP;
extern const IconMask LITE_ICON_LOOP_ONE;
extern const IconMask LITE_ICON_MOON;

void lite_icon_draw_scaled(Canvas *canvas, const IconMask *mask, Rect area, Color color);
