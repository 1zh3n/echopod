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
    SETTINGS_TRANSITION_MS = 140,
    SETTINGS_TRANSITION_PX = 24,
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
    SETTINGS_DOT_X = 254,
    SETTINGS_DOT_Y = 8,
    SETTINGS_DOT_GAP = 12
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
    {COLOR_RGB(255, 159, 28), COLOR_RGB(255, 211, 122), COLOR_RGB(200, 106, 0),
     &LITE_ICON_CATEGORY_LIBRARY},
    {COLOR_RGB(245, 158, 11), COLOR_RGB(252, 211, 77), COLOR_RGB(180, 83, 9),
     &LITE_ICON_CATEGORY_BUTTONS},
    {COLOR_RGB(139, 92, 246), COLOR_RGB(196, 167, 250), COLOR_RGB(91, 33, 182), NULL},
};

static const char CHEVRON[] = ">";

static bool settings_scene_update(void *context, uint32_t now_ms)
{
    SettingsScreens *screens = context;
    screens->menu.list.visible_rows = theme_is_portrait(screens->theme) ? 7 : 5;
    if (screens->transitioning) {
        uint32_t duration = settings_animation_ms(screens->context.store->settings.options,
                                                  SETTINGS_TRANSITION_MS);
        if (now_ms - screens->transition_started_ms >= duration) {
            screens->transitioning = false;
        }
    }
    return true;
}

static void draw_category_dots(Canvas *canvas, const Theme *theme, int group, int x, int y)
{
    for (int index = 0; index < SETTINGS_GROUP_COUNT; index++) {
        Point center = {x + index * SETTINGS_DOT_GAP, y};
        if (index == group) {
            lite_shape_fill_circle(canvas, center, 4, theme->colors->text);
            lite_shape_fill_circle(canvas, center, 2, CATEGORIES[index].base);
        } else {
            lite_shape_fill_circle(canvas, center, 3, CATEGORIES[index].base);
        }
    }
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

static void draw_scene(Canvas *canvas, MenuScreen *menu, int group, int dx)
{
    const Theme *theme = menu->config.theme;
    Canvas shifted = canvas_shifted(canvas, dx, theme_content_area(theme));
    canvas = &shifted;
    const ThemeColors *colors = theme->colors;
    if (group < 0 || group >= SETTINGS_GROUP_COUNT) {
        group = SETTINGS_SOUND;
    }
    const CategoryVisual *visual = &CATEGORIES[group];
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
        draw_category_dots(canvas, theme, group, 112, 18);
        canvas_fill_rect(canvas, (Rect){12, 34, 36, 4}, visual->base);
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
        draw_category_dots(canvas, theme, group, SETTINGS_DOT_X, SETTINGS_DOT_Y);
        row_height = theme->metrics.row_height;
        visible = menu->list.visible_rows;
        if (visible <= 0) {
            visible = SETTINGS_TOP_COUNT;
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
            .base = visual->base,
            .light = visual->light,
            .dark = visual->dark,
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
}

static int transition_distance(uint32_t elapsed, uint32_t duration)
{
    if (duration == 0 || elapsed >= duration) {
        return SETTINGS_TRANSITION_PX;
    }
    uint64_t remaining = duration - elapsed;
    uint64_t left = (uint64_t)SETTINGS_TRANSITION_PX * remaining * remaining * remaining /
                    ((uint64_t)duration * duration * duration);
    return SETTINGS_TRANSITION_PX - (int)left;
}

static bool settings_scene_draw(void *context, Canvas *canvas, MenuScreen *menu)
{
    SettingsScreens *screens = context;
    if (!screens->transitioning) {
        draw_scene(canvas, menu, screens->shown, 0);
        return true;
    }
    uint32_t duration =
        settings_animation_ms(screens->context.store->settings.options, SETTINGS_TRANSITION_MS);
    uint32_t elapsed = screens->menu.now_ms - screens->transition_started_ms;
    if (duration == 0 || elapsed >= duration) {
        screens->transitioning = false;
        draw_scene(canvas, menu, screens->shown, 0);
        return true;
    }
    int moved = transition_distance(elapsed, duration);
    screens->previous_menu.now_ms = screens->menu.now_ms;
    draw_scene(canvas, &screens->previous_menu, screens->previous_group, -moved);
    draw_scene(canvas, menu, screens->shown, SETTINGS_TRANSITION_PX - moved);
    return true;
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
