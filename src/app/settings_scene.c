#include "settings_scene.h"

#include <stddef.h>

#include "gfx/text.h"
#include "ui/label.h"
#include "ui/lite_badge.h"
#include "ui/lite_color_icons.h"
#include "ui/lite_frame.h"

enum {
    SETTINGS_ICON_X = 8,
    SETTINGS_ICON_Y = 95,
    SETTINGS_ICON_RADIUS = 62,
    SETTINGS_GEAR_LEFT = 9,
    SETTINGS_GEAR_TOP = 65,
    SETTINGS_GEAR_SIZE = 60,
    SETTINGS_TEXT_X = 84,
    SETTINGS_TEXT_WIDTH = 200,
    SETTINGS_CHEVRON_RIGHT = 296,
    SETTINGS_CARD_X = 8,
    SETTINGS_CARD_WIDTH = 304,
    SETTINGS_CARD_TOP = 28,
    SETTINGS_CARD_HEIGHT = 134,
    SETTINGS_CARD_BORDER = 2,
    SETTINGS_DIVIDER_X = 24,
    SETTINGS_DIVIDER_WIDTH = 280
};

static const char CHEVRON[] = ">";

static bool settings_scene_update(void *context, uint32_t now_ms)
{
    (void)context;
    (void)now_ms;
    return true;
}

static bool settings_scene_draw(void *context, Canvas *canvas, MenuScreen *menu)
{
    (void)context;
    const Theme *theme = menu->config.theme;
    const ThemeColors *colors = theme->colors;
    Rect rows = lite_list_frame(canvas, theme, menu->config.title);
    int count = menu->model.count(menu->model.context);
    int selected = menu->list.selected;
    int row_height = theme->metrics.row_height;

    int selected_y = rows.y + selected * row_height;
    canvas_fill_rect(canvas,
                     (Rect){SETTINGS_CARD_X, selected_y, SETTINGS_CARD_WIDTH, row_height},
                     colors->selection_background);

    for (int index = 1; index < count; index++) {
        int y = rows.y + index * row_height;
        canvas_fill_rect(canvas,
                         (Rect){SETTINGS_DIVIDER_X, y, SETTINGS_DIVIDER_WIDTH, 1},
                         colors->divider);
    }

    LiteBadgeStyle badge = {
        .base = COLOR_RGB(139, 92, 246),
        .light = COLOR_RGB(196, 167, 250),
        .dark = COLOR_RGB(91, 33, 182),
        .ink = colors->text,
        .shadow = colors->text,
    };
    lite_badge_draw_half(canvas, (Point){SETTINGS_ICON_X, SETTINGS_ICON_Y},
                         SETTINGS_ICON_RADIUS, &badge);
    color_icon_draw_glyph_scaled(
        canvas, &LITE_COLOR_ICON_GEAR,
        (Rect){SETTINGS_GEAR_LEFT, SETTINGS_GEAR_TOP, SETTINGS_GEAR_SIZE, SETTINGS_GEAR_SIZE},
        (Rect){13, 13, 38, 38});

    TextStyle label_style = {.font = theme->fonts.body};
    TextStyle chevron_style = {.font = theme->fonts.small};
    for (int index = 0; index < count; index++) {
        int y = rows.y + index * row_height;
        bool is_selected = index == selected;
        label_style.color = is_selected ? colors->selection_text : colors->text;
        label_draw(canvas, label_style,
                   (Label){.area = {SETTINGS_TEXT_X, y, SETTINGS_TEXT_WIDTH, row_height},
                           .text = menu->model.label(menu->model.context, index)});
        if (menu->model.opens != NULL && !menu->model.opens(menu->model.context, index)) {
            continue;
        }
        chevron_style.color = is_selected ? colors->selection_text : colors->chevron;
        int width = text_width(chevron_style.font, CHEVRON);
        label_draw(canvas, chevron_style,
                   (Label){.area = {SETTINGS_CHEVRON_RIGHT - width, y, width, row_height},
                           .text = CHEVRON});
    }

    canvas_fill_rect(canvas,
                     (Rect){SETTINGS_CARD_X, SETTINGS_CARD_TOP, SETTINGS_CARD_BORDER,
                            SETTINGS_CARD_HEIGHT},
                     colors->text);
    canvas_fill_rect(canvas,
                     (Rect){SETTINGS_CARD_X + SETTINGS_CARD_WIDTH - SETTINGS_CARD_BORDER,
                            SETTINGS_CARD_TOP, SETTINGS_CARD_BORDER, SETTINGS_CARD_HEIGHT},
                     colors->text);
    return true;
}

MenuScene *settings_scene(void)
{
    static MenuScene scene = {settings_scene_update, settings_scene_draw, NULL, true};
    return &scene;
}
