#pragma once

#include <stdint.h>

#include "gfx/canvas.h"
#include "ui/theme.h"

#ifdef ECHO_LITE
#define LITE_ICON_POINTER(offset) ((const uint8_t *)(uintptr_t)(0x10000000u + (offset)))
#define LITE_ICON_POINTER_BASE 0x10000000u
#define LITE_ICON_POINTER_LIMIT (LITE_ICON_POINTER_BASE + 16930u)
const uint8_t *lite_icon_levels(const uint8_t *encoded, uint32_t bytes);
#endif

extern const IconMask LITE_ICON_SHUFFLE;
extern const IconMask LITE_ICON_LOOP;
extern const IconMask LITE_ICON_LOOP_ONE;
extern const IconMask LITE_ICON_MOON;
extern const IconMask LITE_ICON_SHUFFLE_18;
extern const IconMask LITE_ICON_LOOP_18;
extern const IconMask LITE_ICON_LOOP_ONE_18;
extern const IconMask LITE_ICON_PLAY_18;
extern const IconMask LITE_ICON_PAUSE_18;
extern const IconMask LITE_ICON_BLUETOOTH;
extern const IconMask LITE_ICON_CATEGORY_SOUND;
extern const IconMask LITE_ICON_CATEGORY_DISPLAY;
extern const IconMask LITE_ICON_CATEGORY_LIBRARY;
extern const IconMask LITE_ICON_CATEGORY_BUTTONS;

void lite_icon_draw_scaled(Canvas *canvas, const IconMask *mask, Rect area, Color color);