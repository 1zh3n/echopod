#include "ui/lite_frame.h"

#ifdef ECHO_LITE
#include "ui/label.h"

Rect lite_list_frame(Canvas *canvas, const Theme *theme, const char *title)
{
    const ThemeColors *colors = theme->colors;
    Rect content = theme_content_area(theme);
    canvas_fill_rect(canvas, content, colors->background);
    TextStyle style = {.font = theme->fonts.title, .color = colors->text};
    label_draw(canvas, style,
               (Label){.area = {16, 8, content.width - 32, 20}, .text = title});
    canvas_fill_rect(canvas, (Rect){16, 25, content.width - 32, 2}, colors->text);
    canvas_fill_rect(canvas, (Rect){12, 32, content.width - 16, 134}, colors->text);
    Rect card = {8, 28, content.width - 16, 134};
    canvas_fill_rect(canvas, card, colors->header_background);
    canvas_fill_rect(canvas, (Rect){card.x, card.y, card.width, 2}, colors->text);
    canvas_fill_rect(canvas, (Rect){card.x, card.y + card.height - 2, card.width, 2}, colors->text);
    canvas_fill_rect(canvas, (Rect){card.x, card.y, 2, card.height}, colors->text);
    canvas_fill_rect(canvas, (Rect){card.x + card.width - 2, card.y, 2, card.height}, colors->text);
    return (Rect){12, 30, card.width - 8, 130};
}
#endif
