#include <stddef.h>

#include "ui/lite_badge.h"

enum {
    BADGE_BORDER = 2,
    BADGE_RIM = 2,
    BADGE_SHADOW = 4
};

static int circle_half_width(int radius, int dy)
{
    int value = radius * radius - dy * dy;
    if (value <= 0) {
        return 0;
    }
    int low = 0;
    int high = radius;
    while (low <= high) {
        int middle = (low + high) / 2;
        if (middle * middle <= value) {
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }
    return high;
}

static void fill_span(Canvas *canvas, int x, int y, int width, Color color)
{
    if (width > 0) {
        canvas_fill_rect(canvas, (Rect){x, y, width, 1}, color);
    }
}

static void draw_circle_fill(Canvas *canvas, Point center, int radius, Color color)
{
    for (int dy = -radius; dy <= radius; dy++) {
        int width = circle_half_width(radius, dy);
        fill_span(canvas, center.x - width, center.y + dy, width * 2 + 1, color);
    }
}

static void draw_half_fill(Canvas *canvas, Point center, int radius, Color color)
{
    for (int dy = -radius; dy <= radius; dy++) {
        int width = circle_half_width(radius, dy);
        fill_span(canvas, center.x, center.y + dy, width + 1, color);
    }
}

static void draw_circle(Canvas *canvas, Point center, int radius, const LiteBadgeStyle *style)
{
    draw_circle_fill(canvas, (Point){center.x + BADGE_SHADOW, center.y + BADGE_SHADOW}, radius,
                     style->shadow);
    draw_circle_fill(canvas, center, radius, style->ink);
    int base = radius - BADGE_BORDER;
    draw_circle_fill(canvas, center, base, style->base);
    int inner = base - BADGE_RIM;
    for (int dy = -base; dy <= base; dy++) {
        int outer_width = circle_half_width(base, dy);
        int inner_width = circle_half_width(inner, dy);
        if (dy < 0) {
            fill_span(canvas, center.x - outer_width, center.y + dy,
                      outer_width - inner_width, style->light);
        } else if (dy > 0) {
            fill_span(canvas, center.x + inner_width, center.y + dy,
                      outer_width - inner_width, style->dark);
        }
    }
}

static void draw_half(Canvas *canvas, Point center, int radius, const LiteBadgeStyle *style)
{
    draw_half_fill(canvas, (Point){center.x + BADGE_SHADOW, center.y + BADGE_SHADOW}, radius,
                   style->shadow);
    draw_half_fill(canvas, center, radius, style->ink);
    int base = radius - BADGE_BORDER;
    draw_half_fill(canvas, center, base, style->base);
    int inner = base - BADGE_RIM;
    for (int dy = -base; dy <= base; dy++) {
        int outer_width = circle_half_width(base, dy);
        int inner_width = circle_half_width(inner, dy);
        if (dy < 0) {
            fill_span(canvas, center.x + inner_width, center.y + dy,
                      outer_width - inner_width, style->light);
        } else if (dy > 0) {
            fill_span(canvas, center.x + inner_width, center.y + dy,
                      outer_width - inner_width, style->dark);
        }
    }
}

static void draw_small_circle(Canvas *canvas, Point center, int radius, Color color)
{
    static const uint8_t ROWS_2[] = {3, 5, 5, 5, 3};
    static const uint8_t ROWS_3[] = {3, 5, 7, 7, 7, 5, 3};
    static const uint8_t ROWS_4[] = {3, 5, 7, 9, 9, 9, 7, 5, 3};
    const uint8_t *rows = NULL;
    int count = 0;
    switch (radius) {
    case 2:
        rows = ROWS_2;
        count = (int)(sizeof ROWS_2 / sizeof ROWS_2[0]);
        break;
    case 3:
        rows = ROWS_3;
        count = (int)(sizeof ROWS_3 / sizeof ROWS_3[0]);
        break;
    case 4:
        rows = ROWS_4;
        count = (int)(sizeof ROWS_4 / sizeof ROWS_4[0]);
        break;
    default:
        draw_circle_fill(canvas, center, radius, color);
        return;
    }
    int top = center.y - count / 2;
    for (int index = 0; index < count; index++) {
        int width = rows[index];
        fill_span(canvas, center.x - width / 2, top + index, width, color);
    }
}

void lite_shape_fill_circle(Canvas *canvas, Point center, int radius, Color color)
{
    if (canvas == NULL || radius <= 0) {
        return;
    }
    draw_small_circle(canvas, center, radius, color);
}

void lite_badge_draw_circle(Canvas *canvas, Point center, int radius,
                            const LiteBadgeStyle *style)
{
    if (canvas == NULL || style == NULL || radius <= BADGE_BORDER + BADGE_RIM) {
        return;
    }
    draw_circle(canvas, center, radius, style);
}

void lite_badge_draw_half(Canvas *canvas, Point center, int radius,
                          const LiteBadgeStyle *style)
{
    if (canvas == NULL || style == NULL || radius <= BADGE_BORDER + BADGE_RIM) {
        return;
    }
    draw_half(canvas, center, radius, style);
}
