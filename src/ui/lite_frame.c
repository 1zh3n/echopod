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
    return lite_theme_is_dark(theme) ? COLOR_RGB(82, 82, 91) : theme->colors->text;
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

Rect lite_list_frame(Canvas *canvas, const Theme *theme, const char *title)
{
    const ThemeColors *colors = theme->colors;
    Rect content = theme_content_area(theme);
    canvas_fill_rect(canvas, content, colors->background);
    TextStyle style = {.font = theme->fonts.title, .color = colors->text};
    if (theme_is_portrait(theme)) {
        label_draw(canvas, style, (Label){.area = {12, 8, content.width - 24, 20}, .text = title});
        canvas_fill_rect(canvas, (Rect){12, 30, content.width - 24, lite_card_border_width(theme)},
                         lite_card_border_color(theme));
        lite_card(canvas, theme, (Rect){8, 40, content.width - 16, 266}, 4);
        return (Rect){12, 44, content.width - 24, 258};
    }
    label_draw(canvas, style,
               (Label){.area = {16, 8, content.width - 32, 20}, .text = title});
    canvas_fill_rect(canvas, (Rect){16, 25, content.width - 32, lite_card_border_width(theme)},
                     lite_card_border_color(theme));
    lite_card(canvas, theme, (Rect){8, 28, content.width - 16, 134}, 4);
    return (Rect){12, 30, content.width - 24, 130};
}
#endif
