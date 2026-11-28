#include "settings_scene.h"

#ifdef ECHO_LITE

#include <stddef.h>

#include "gfx/text.h"
#include "settings/settings.h"
#include "settings_screens.h"
#include "ui/label.h"
#include "ui/lite_badge.h"
#include "ui/lite_color_icons.h"
#include "ui/lite_frame.h"
#include "ui/lite_icons.h"

enum {
    SETTINGS_ICON_X = 8,
    SETTINGS_ICON_Y = 95,
    SETTINGS_ICON_RADIUS = 62,
    SETTINGS_CATEGORY_LEFT = 12,
    SETTINGS_CATEGORY_TOP = 68,
    SETTINGS_CATEGORY_SIZE = 54,
    SETTINGS_GEAR_LEFT = 9,
    SETTINGS_GEAR_TOP = 65,
    SETTINGS_GEAR_SIZE = 60,
    SETTINGS_TEXT_X = 84,
    SETTINGS_CHEVRON_RIGHT = 296,
    SETTINGS_CARD_X = 8,
    SETTINGS_CARD_WIDTH = 304,
    SETTINGS_CARD_TOP = 28,
    SETTINGS_CARD_HEIGHT = 134,
    SETTINGS_CARD_BORDER = 2,
    SETTINGS_DIVIDER_X = 24,
    SETTINGS_DIVIDER_WIDTH = 280,
    HOME_CARD_X = 8,
    HOME_CARD_WIDTH = 304,
    HOME_CARD_TOP = 30,
    HOME_CARD_HEIGHT = 23,
    HOME_CARD_GAP = 3,
    HOME_PORTRAIT_X = 8,
    HOME_PORTRAIT_WIDTH = 154,
    HOME_PORTRAIT_TOP = 58,
    HOME_PORTRAIT_HEIGHT = 44,
    HOME_PORTRAIT_GAP = 4
};

typedef struct CategoryVisual {
    Color base;
    Color light;
    Color dark;
    const IconMask *glyph;
} CategoryVisual;

static const CategoryVisual CATEGORIES[SETTINGS_GROUP_COUNT] = {
    {COLOR_RGB(47, 168, 255), COLOR_RGB(139, 215, 255), COLOR_RGB(22, 119, 200),
     &LITE_ICON_CATEGORY_SOUND},
    {COLOR_RGB(53, 199, 232), COLOR_RGB(139, 227, 245), COLOR_RGB(14, 116, 144),
     &LITE_ICON_CATEGORY_DISPLAY},
    {COLOR_RGB(139, 92, 246), COLOR_RGB(196, 167, 250), COLOR_RGB(91, 33, 182), NULL},
};

static const Color HOME_COLORS[5] = {
    COLOR_RGB(47, 168, 255), COLOR_RGB(99, 102, 241), COLOR_RGB(47, 168, 255),
    COLOR_RGB(53, 199, 232), COLOR_RGB(139, 92, 246),
};

static const char CHEVRON[] = ">";

static void draw_card_border(Canvas *canvas, Rect card, Color color)
{
    canvas_fill_rect(canvas, (Rect){card.x, card.y, card.width, 2}, color);
    canvas_fill_rect(canvas, (Rect){card.x, card.y + card.height - 2, card.width, 2}, color);
    canvas_fill_rect(canvas, (Rect){card.x, card.y, 2, card.height}, color);
    canvas_fill_rect(canvas, (Rect){card.x + card.width - 2, card.y, 2, card.height}, color);
}

static bool home_scene_update(void *context, uint32_t now_ms)
{
    (void)now_ms;
    SettingsScreens *screens = context;
    screens->home.list.visible_rows = 5;
    return true;
}

static bool home_scene_draw(void *context, Canvas *canvas, MenuScreen *menu)
{
    (void)context;
    const Theme *theme = menu->config.theme;
    const ThemeColors *colors = theme->colors;
    bool portrait = theme_is_portrait(theme);
    canvas_fill_rect(canvas, theme_content_area(theme), colors->background);
    int x = portrait ? HOME_PORTRAIT_X : HOME_CARD_X;
    int width = portrait ? HOME_PORTRAIT_WIDTH : HOME_CARD_WIDTH;
    int top = portrait ? HOME_PORTRAIT_TOP : HOME_CARD_TOP;
    int height = portrait ? HOME_PORTRAIT_HEIGHT : HOME_CARD_HEIGHT;
    int gap = portrait ? HOME_PORTRAIT_GAP : HOME_CARD_GAP;
    TextStyle label_style = {.font = theme->fonts.body};
    TextStyle value_style = {.font = theme->fonts.small};
    for (int index = 0; index < 5; index++) {
        Rect card = {x, top + index * (height + gap), width, height};
        lite_card(canvas, theme, card, 2);
        bool selected = index == menu->list.selected;
        if (selected) {
            canvas_fill_rect(canvas,
                             (Rect){card.x + 2, card.y + 2, card.width - 4, card.height - 4},
                             colors->selection_background);
            draw_card_border(canvas, card, colors->selection_background);
        }
        lite_shape_fill_circle(canvas,
                               (Point){card.x + 12, card.y + card.height / 2},
                               portrait ? 7 : 5, HOME_COLORS[index]);
        label_style.color = selected ? colors->selection_text : colors->text;
        int label_x = card.x + 24;
        int label_height = portrait ? 20 : height;
        int label_y = portrait ? card.y + 6 : card.y;
        label_draw(canvas, label_style,
                   (Label){.area = {label_x, label_y, card.width - 60, label_height},
                           .text = menu->model.label(menu->model.context, index)});
        const char *value = menu->model.detail != NULL
                                ? menu->model.detail(menu->model.context, index)
                                : NULL;
        bool opens = menu->model.opens != NULL &&
                     menu->model.opens(menu->model.context, index);
        value_style.color = selected ? colors->selection_text : colors->text_secondary;
        if (value != NULL) {
            int value_width = text_width(value_style.font, value);
            Rect area = {card.x + card.width - value_width - 10,
                         portrait ? card.y + 24 : card.y + (height - 16) / 2, value_width, 16};
            label_draw(canvas, value_style, (Label){.area = area, .text = value});
        } else if (opens) {
            int chevron_width = text_width(value_style.font, CHEVRON);
            Rect area = {card.x + card.width - chevron_width - 10,
                         portrait ? card.y + 24 : card.y + (height - 16) / 2, chevron_width, 16};
            value_style.color = selected ? colors->selection_text : colors->chevron;
            label_draw(canvas, value_style, (Label){.area = area, .text = CHEVRON});
        }
    }
    return true;
}

static bool settings_scene_update(void *context, uint32_t now_ms)
{
    (void)now_ms;
    SettingsScreens *screens = context;
    screens->menu.list.visible_rows = theme_is_portrait(screens->theme) ? 7 : SETTINGS_LIST_ROWS;
    return true;
}

static void draw_category_glyph(Canvas *canvas, int group)
{
    if (CATEGORIES[group].glyph != NULL) {
        lite_icon_draw_scaled(
            canvas, CATEGORIES[group].glyph,
            (Rect){SETTINGS_CATEGORY_LEFT, SETTINGS_CATEGORY_TOP, SETTINGS_CATEGORY_SIZE,
                   SETTINGS_CATEGORY_SIZE},
            COLOR_RGB(255, 255, 255));
        return;
    }
    color_icon_draw_glyph_scaled(
        canvas, &LITE_COLOR_ICON_GEAR,
        (Rect){SETTINGS_GEAR_LEFT, SETTINGS_GEAR_TOP, SETTINGS_GEAR_SIZE, SETTINGS_GEAR_SIZE},
        (Rect){13, 13, 38, 38});
}

static bool settings_scene_draw(void *context, Canvas *canvas, MenuScreen *menu)
{
    SettingsScreens *screens = context;
    const Theme *theme = menu->config.theme;
    const ThemeColors *colors = theme->colors;
    int group = screens->shown;
    if (group < 0 || group >= SETTINGS_GROUP_COUNT) {
        group = SETTINGS_SOUND;
    }
    CategoryVisual visual = CATEGORIES[group];
    int count = menu->model.count(menu->model.context);
    int selected = menu->list.selected;
    int top = menu->list.top;
    bool portrait = theme_is_portrait(theme);
    Rect rows;
    int row_height;
    int visible;
    int fill_x;
    int fill_width;
    int divider_x;
    int divider_width;
    int border = lite_card_border_width(theme);
    Color border_color = lite_card_border_color(theme);

    if (portrait) {
        canvas_fill_rect(canvas, theme_content_area(theme), colors->background);
        TextStyle title_style = {.font = theme->fonts.title, .color = colors->text};
        label_draw(canvas, title_style,
                   (Label){.area = {12, 8, 146, 20}, .text = settings_category_title(group)});
        canvas_fill_rect(canvas, (Rect){12, 34, 36, 4}, visual.base);
        rows = lite_card(canvas, theme, (Rect){8, 58, 154, 254}, 4);
        row_height = count > 6 ? 34 : 38;
        visible = count < 7 ? count : 7;
        fill_x = 10;
        fill_width = 150;
        divider_x = 20;
        divider_width = 130;
    } else {
        canvas_fill_rect(canvas, theme_content_area(theme), colors->background);
        rows = lite_card(canvas, theme,
                         (Rect){SETTINGS_CARD_X, SETTINGS_CARD_TOP, SETTINGS_CARD_WIDTH,
                                SETTINGS_CARD_HEIGHT},
                         4);
        row_height = theme->metrics.row_height;
        visible = menu->list.visible_rows;
        if (visible <= 0) {
            visible = SETTINGS_LIST_ROWS;
        }
        fill_x = SETTINGS_CARD_X;
        fill_width = SETTINGS_CARD_WIDTH;
        divider_x = SETTINGS_DIVIDER_X;
        divider_width = SETTINGS_DIVIDER_WIDTH;
    }

    int selected_position = selected - top;
    if (selected_position >= 0 && selected_position < visible) {
        int selected_y = rows.y + selected_position * row_height;
        canvas_fill_rect(canvas, (Rect){fill_x, selected_y, fill_width, row_height},
                         colors->selection_background);
    }

    for (int position = 1; position < visible; position++) {
        int y = rows.y + position * row_height;
        canvas_fill_rect(canvas, (Rect){divider_x, y, divider_width, 1}, colors->divider);
    }

    if (!portrait) {
        LiteBadgeStyle badge = {
            .base = visual.base,
            .light = visual.light,
            .dark = visual.dark,
            .ink = colors->text,
            .shadow = lite_card_shadow_color(theme),
        };
        lite_badge_draw_half(canvas, (Point){SETTINGS_ICON_X, SETTINGS_ICON_Y},
                             SETTINGS_ICON_RADIUS, &badge);
        draw_category_glyph(canvas, group);
    }

    TextStyle label_style = {.font = theme->fonts.body};
    TextStyle value_style = {.font = theme->fonts.small};
    TextStyle chevron_style = {.font = theme->fonts.small};
    int chevron_width = text_width(chevron_style.font, CHEVRON);
    for (int position = 0; position < visible; position++) {
        int index = top + position;
        if (index >= count) {
            break;
        }
        int y = rows.y + position * row_height;
        bool is_selected = index == selected;
        const char *value =
            menu->model.detail != NULL ? menu->model.detail(menu->model.context, index) : NULL;
        label_style.color = is_selected ? colors->selection_text : colors->text;
        if (portrait) {
            int half = row_height / 2;
            label_draw(canvas, label_style,
                       (Label){.area = {20, y, 130, half},
                               .text = menu->model.label(menu->model.context, index)});
            if (value != NULL) {
                value_style.color = is_selected ? colors->selection_text : colors->text_secondary;
                int width = text_width(value_style.font, value);
                label_draw(canvas, value_style,
                           (Label){.area = {150 - width, y + half, width, half}, .text = value});
            }
            continue;
        }
        bool opens = menu->model.opens != NULL && menu->model.opens(menu->model.context, index);
        int right = SETTINGS_CHEVRON_RIGHT;
        if (opens) {
            right -= chevron_width + theme->metrics.detail_gap;
        }
        int value_width = value != NULL ? text_width(value_style.font, value) : 0;
        int value_left = right - value_width;
        int label_width = value_left - SETTINGS_TEXT_X - 4;
        if (label_width < 0) {
            label_width = 0;
        }
        label_draw(canvas, label_style,
                   (Label){.area = {SETTINGS_TEXT_X, y, label_width, row_height},
                           .text = menu->model.label(menu->model.context, index)});
        if (value != NULL) {
            value_style.color = is_selected ? colors->selection_text : colors->text_secondary;
            label_draw(canvas, value_style,
                       (Label){.area = {value_left, y, value_width, row_height}, .text = value});
        }
        if (!opens) {
            continue;
        }
        chevron_style.color = is_selected ? colors->selection_text : colors->chevron;
        label_draw(canvas, chevron_style,
                   (Label){.area = {SETTINGS_CHEVRON_RIGHT - chevron_width, y, chevron_width,
                                    row_height},
                           .text = CHEVRON});
    }

    if (count > visible) {
        int track_x = portrait ? 156 : SETTINGS_CARD_X + SETTINGS_CARD_WIDTH - 6;
        int track_height = rows.height;
        int thumb_height = track_height * visible / count;
        if (thumb_height < 8) {
            thumb_height = 8;
        }
        int hidden = count - visible;
        int thumb_y = rows.y + (track_height - thumb_height) * top / hidden;
        canvas_fill_rect(canvas, (Rect){track_x, thumb_y, 2, thumb_height},
                         colors->scrollbar_thumb);
    }

    if (!portrait) {
        canvas_fill_rect(canvas,
                         (Rect){SETTINGS_CARD_X, SETTINGS_CARD_TOP, border, SETTINGS_CARD_HEIGHT},
                         border_color);
        canvas_fill_rect(canvas,
                         (Rect){SETTINGS_CARD_X + SETTINGS_CARD_WIDTH - border, SETTINGS_CARD_TOP,
                                border, SETTINGS_CARD_HEIGHT},
                         border_color);
    }
    return true;
}

MenuScene *settings_home_scene(struct SettingsScreens *screens)
{
    static MenuScene scene;
    scene.update = home_scene_update;
    scene.draw = home_scene_draw;
    scene.context = screens;
    scene.on = true;
    return &scene;
}

MenuScene *settings_scene(struct SettingsScreens *screens)
{
    static MenuScene scene;
    scene.update = settings_scene_update;
    scene.draw = settings_scene_draw;
    scene.context = screens;
    scene.on = true;
    return &scene;
}

#else

typedef int SettingsSceneTranslationUnit;

#endif
