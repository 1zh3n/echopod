#pragma once

#include "gfx/canvas.h"
#include "ui/theme.h"

extern const IconMask LITE_ICON_SHUFFLE;
extern const IconMask LITE_ICON_LOOP;
extern const IconMask LITE_ICON_LOOP_ONE;
extern const IconMask LITE_ICON_MOON;
extern const IconMask LITE_ICON_SHUFFLE_18;
extern const IconMask LITE_ICON_LOOP_18;
extern const IconMask LITE_ICON_LOOP_ONE_18;
extern const IconMask LITE_ICON_PLAY_18;
extern const IconMask LITE_ICON_PAUSE_18;

void lite_icon_draw_scaled(Canvas *canvas, const IconMask *mask, Rect area, Color color);
