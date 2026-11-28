#include "ui/lite_frame.h"

#ifdef ECHO_LITE
#include "ui/label.h"
#include "ui/lite_status.h"

bool lite_theme_is_dark(const Theme *theme)
{
    return theme->colors->background == COLOR_RGB(11, 11, 13);
}

Color lite_card_border_color(const Theme *theme)
{
    return lite_theme_is_dark(theme) ? COLOR_RGB(82, 82, 91) : theme->colors->text;
}

int lite_card_border_width(const Theme *theme)
{
    return lite_theme_is_dark(theme) ? 1 : 2;
}

Color lite_card_shadow_color(const Theme *theme)
{
    return lite_theme_is_dark(theme) ? COLOR_RGB(0, 0, 0) : theme->colors->text;
}

int lite_list_rows(const Theme *theme)
{
    return theme_is_portrait(theme) ? 9 : 5;
}

Rect lite_card(Canvas *canvas, const Theme *theme, Rect card, int shadow)
{
    int border = lite_card_border_width(theme);
    Color edge = lite_card_border_color(theme);
    canvas_fill_rect(canvas, (Rect){card.x + shadow, card.y + shadow, card.width, card.height},
                     lite_status_shadow());
    canvas_fill_rect(canvas, card, theme->colors->header_background);
    canvas_fill_rect(canvas, (Rect){card.x, card.y, card.width, border}, edge);
    canvas_fill_rect(canvas, (Rect){card.x, card.y + card.height - border, card.width, border},
                     edge);
    canvas_fill_rect(canvas, (Rect){card.x, card.y, border, card.height}, edge);
    canvas_fill_rect(canvas, (Rect){card.x + card.width - border, card.y, border, card.height},
                     edge);
    return (Rect){card.x + 4, card.y + 2, card.width - 8, card.height - 4};
}

static int integer_sqrt(int value)
{
    int root = 0;
    while ((root + 1) * (root + 1) <= value) {
        root++;
    }
    return root;
}

static int round_inset(int radius, int row)
{
    int dy = radius - 1 - row;
    return radius - integer_sqrt(radius * radius - dy * dy);
}

static void fill_round_rect(Canvas *canvas, Rect rect, int radius, Color color)
{
    if (rect.width <= 0 || rect.height <= 0) {
        return;
    }
    if (radius <= 0 || rect.width < 2 * radius || rect.height < 2 * radius) {
        canvas_fill_rect(canvas, rect, color);
        return;
    }
    canvas_fill_rect(canvas, (Rect){rect.x + radius, rect.y, rect.width - 2 * radius,
                                    rect.height},
                     color);
    canvas_fill_rect(canvas, (Rect){rect.x, rect.y + radius, rect.width,
                                    rect.height - 2 * radius},
                     color);
    for (int row = 0; row < radius; row++) {
        int inset = round_inset(radius, row);
        int width = rect.width - 2 * inset;
        canvas_fill_rect(canvas, (Rect){rect.x + inset, rect.y + row, width, 1}, color);
        canvas_fill_rect(canvas,
                         (Rect){rect.x + inset, rect.y + rect.height - 1 - row, width, 1},
                         color);
    }
}

void lite_card_rounded(Canvas *canvas, const Theme *theme, Rect card, int shadow, int radius)
{
    int border = lite_card_border_width(theme);
    fill_round_rect(canvas,
                    (Rect){card.x + shadow, card.y + shadow, card.width, card.height}, radius,
                    lite_status_shadow());
    fill_round_rect(canvas, card, radius, lite_card_border_color(theme));
    fill_round_rect(canvas,
                    (Rect){card.x + border, card.y + border, card.width - 2 * border,
                           card.height - 2 * border},
                    radius > border ? radius - border : 1, theme->colors->header_background);
}

void lite_status_block_draw(Canvas *canvas, Point origin, Color color)
{
    canvas_fill_rect(canvas, (Rect){origin.x + 1, origin.y + 1, 10, 10}, lite_status_shadow());
    canvas_fill_rect(canvas, (Rect){origin.x, origin.y, 10, 10}, color);
}

void lite_status_root_draw(Canvas *canvas, Point origin, Color all, Color favorites)
{
    canvas_fill_rect(canvas, (Rect){origin.x + 1, origin.y + 1, 28, 10}, lite_status_shadow());
    canvas_fill_rect(canvas, (Rect){origin.x, origin.y, 13, 10}, all);
    canvas_fill_rect(canvas, (Rect){origin.x + 15, origin.y, 13, 10}, favorites);
}

Rect lite_list_frame_no_title(Canvas *canvas, const Theme *theme)
{
    const ThemeColors *colors = theme->colors;
    Rect content = theme_content_area(theme);
    canvas_fill_rect(canvas, content, colors->background);
    if (theme_is_portrait(theme)) {
        lite_card(canvas, theme, (Rect){8, 28, content.width - 16, 280}, 4);
        return (Rect){12, 30, content.width - 24, 276};
    }
    lite_card(canvas, theme, (Rect){8, 28, content.width - 16, 134}, 4);
    return (Rect){12, 30, content.width - 24, 130};
}

Rect lite_list_frame(Canvas *canvas, const Theme *theme, const char *title)
{
    const ThemeColors *colors = theme->colors;
    Rect content = theme_content_area(theme);
    canvas_fill_rect(canvas, content, colors->background);
    TextStyle style = {.font = theme->fonts.title, .color = colors->text};
    if (theme_is_portrait(theme)) {
        label_draw(canvas, style, (Label){.area = {12, 8, content.width - 51, 20}, .text = title});
        canvas_fill_rect(canvas, (Rect){12, 30, content.width - 24, lite_card_border_width(theme)},
                         lite_card_border_color(theme));
        lite_card(canvas, theme, (Rect){8, 40, content.width - 16, 272}, 4);
        return (Rect){12, 44, content.width - 24, 264};
    }
    label_draw(canvas, style,
               (Label){.area = {16, 8, content.width - 55, 20}, .text = title});
    canvas_fill_rect(canvas, (Rect){16, 25, content.width - 32, lite_card_border_width(theme)},
                     lite_card_border_color(theme));
    lite_card(canvas, theme, (Rect){8, 28, content.width - 16, 134}, 4);
    return (Rect){12, 30, content.width - 24, 130};
}
#endif
