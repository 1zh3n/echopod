#pragma once

#include "gfx/canvas.h"

typedef struct LiteBadgeStyle {
    Color base;
    Color light;
    Color dark;
    Color ink;
    Color shadow;
} LiteBadgeStyle;

void lite_badge_draw_circle(Canvas *canvas, Point center, int radius,
                            const LiteBadgeStyle *style);
void lite_badge_draw_half(Canvas *canvas, Point center, int radius,
                          const LiteBadgeStyle *style);
