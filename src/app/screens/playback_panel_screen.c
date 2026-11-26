#include "playback_panel_screen.h"

#ifdef ECHO_LITE

#include <stddef.h>

#include "core/i18n.h"
#include "hal/clock.h"
#include "player.h"
#include "settings_screens.h"
#include "ui/label.h"
#include "ui/lite_icons.h"

enum {
    CARD_X = 12,
    CARD_Y = 30,
    CARD_WIDTH = 140,
    CARD_HEIGHT = 112,
    CARD_GAP = 16,
    CARD_SHADOW = 5,
    CARD_BORDER = 2,
    ICON_SIZE = 48,
    ICON_TOP = 44,
    LABEL_TOP = 98,
    LABEL_HEIGHT = 20,
    VALUE_TOP = 120,
    VALUE_HEIGHT = 16,
    HINT_TOP = 151,
    HINT_HEIGHT = 16
};

static const IconMask *mode_icon(PlaybackMode mode)
{
    switch (mode) {
    case PLAYBACK_RANDOM:
        return &LITE_ICON_SHUFFLE;
    case PLAYBACK_LOOP:
        return &LITE_ICON_LOOP;
    case PLAYBACK_ONE:
        return &LITE_ICON_LOOP_ONE;
    }
    return &LITE_ICON_LOOP;
}

static StringId mode_label(PlaybackMode mode)
{
    switch (mode) {
    case PLAYBACK_RANDOM:
        return STR_MENU_SHUFFLE_SONGS;
    case PLAYBACK_LOOP:
        return STR_MENU_REPEAT_ALL;
    case PLAYBACK_ONE:
        return STR_MENU_REPEAT_ONE;
    }
    return STR_MENU_REPEAT_ALL;
}

static void draw_card(Canvas *canvas, const Theme *theme, Rect area, bool selected)
{
    Color ink = theme->colors->text;
    Color border = selected ? theme->colors->selection_background : ink;
    canvas_fill_rect(canvas,
                     (Rect){area.x + CARD_SHADOW, area.y + CARD_SHADOW, area.width, area.height},
                     ink);
    canvas_fill_rect(canvas, area, theme->colors->header_background);
    canvas_fill_rect(canvas, (Rect){area.x, area.y, area.width, CARD_BORDER}, border);
    canvas_fill_rect(canvas,
                     (Rect){area.x, area.y + area.height - CARD_BORDER, area.width, CARD_BORDER},
                     border);
    canvas_fill_rect(canvas, (Rect){area.x, area.y, CARD_BORDER, area.height}, border);
    canvas_fill_rect(canvas,
                     (Rect){area.x + area.width - CARD_BORDER, area.y, CARD_BORDER, area.height},
                     border);
}

static void draw_icon(Canvas *canvas, const Theme *theme, Rect area, const IconMask *mask,
                      bool selected)
{
    Color color = selected ? theme->colors->selection_background : theme->colors->text_secondary;
    lite_icon_draw_scaled(canvas, mask, area, color);
}

static void draw_text(Canvas *canvas, const Theme *theme, Rect area, StringId text, bool title,
                      bool selected)
{
    TextStyle style = {.font = title ? theme->fonts.title : theme->fonts.small,
                       .color = selected ? theme->colors->text : theme->colors->text_secondary};
    label_draw(canvas, style,
               (Label){.area = label_centered_area(style.font, area, i18n_text(text)),
                       .text = i18n_text(text)});
}

static void draw(void *context, Canvas *canvas)
{
    PlaybackPanelScreen *panel = context;
    const Theme *theme = panel->theme;
    int width = theme->metrics.screen_width;
    int height = theme->metrics.screen_height;
    canvas_fill_rect(canvas, (Rect){0, 0, width, height}, theme->colors->background);

    PlaybackMode mode = player_playback_mode(panel->settings->player);
    int sleep = settings_sleep_current(panel->settings);
    Rect first = {CARD_X, CARD_Y, CARD_WIDTH, CARD_HEIGHT};
    Rect second = {CARD_X + CARD_WIDTH + CARD_GAP, CARD_Y, CARD_WIDTH, CARD_HEIGHT};
    draw_card(canvas, theme, first, panel->selected == 0);
    draw_card(canvas, theme, second, panel->selected == 1);

    draw_icon(canvas, theme,
              (Rect){first.x + (first.width - ICON_SIZE) / 2, ICON_TOP, ICON_SIZE, ICON_SIZE},
              mode_icon(mode), panel->selected == 0);
    draw_icon(canvas, theme,
              (Rect){second.x + (second.width - ICON_SIZE) / 2, ICON_TOP, ICON_SIZE, ICON_SIZE},
              &LITE_ICON_MOON, panel->selected == 1);

    draw_text(canvas, theme, (Rect){first.x, LABEL_TOP, first.width, LABEL_HEIGHT}, mode_label(mode),
              true, panel->selected == 0);
    draw_text(canvas, theme, (Rect){second.x, LABEL_TOP, second.width, LABEL_HEIGHT},
              STR_SETTINGS_SLEEP_TIMER, true, panel->selected == 1);

    TextStyle value = {.font = theme->fonts.small,
                       .color = panel->selected == 0 ? theme->colors->text
                                                     : theme->colors->text_secondary};
    const char *mode_value = "";
    if (mode == PLAYBACK_RANDOM) {
        mode_value = i18n_text(STR_SETTINGS_LIST);
    } else if (mode == PLAYBACK_LOOP) {
        mode_value = i18n_text(STR_SETTINGS_ALL);
    } else {
        mode_value = i18n_text(STR_SETTINGS_ONE);
    }
    label_draw(canvas, value,
               (Label){.area = label_centered_area(value.font,
                                                   (Rect){first.x, VALUE_TOP, first.width,
                                                          VALUE_HEIGHT},
                                                   mode_value),
                       .text = mode_value});

    TextStyle sleep_style = {.font = theme->fonts.small,
                             .color = panel->selected == 1 ? theme->colors->text
                                                           : theme->colors->text_secondary};
    const char *sleep_value = settings_sleep_label(sleep);
    label_draw(canvas, sleep_style,
               (Label){.area = label_centered_area(sleep_style.font,
                                                   (Rect){second.x, VALUE_TOP, second.width,
                                                          VALUE_HEIGHT},
                                                   sleep_value),
                       .text = sleep_value});

    TextStyle hint = {.font = theme->fonts.small, .color = theme->colors->text_tertiary};
    static const char HINT[] = "VOL +/-   PLAY   BACK";
    label_draw(canvas, hint,
               (Label){.area = label_centered_area(hint.font,
                                                   (Rect){0, HINT_TOP, width, HINT_HEIGHT}, HINT),
                       .text = HINT});
}

static ScreenAction handle_key(void *context, KeyEvent event, uint32_t now_ms)
{
    PlaybackPanelScreen *panel = context;
    panel->now_ms = now_ms;
    if (event.key == KEY_MENU) {
        return screen_action_pop();
    }
    if (event.gesture != KEY_GESTURE_PRESS) {
        return screen_action_none();
    }
    switch (event.key) {
    case KEY_VOLUME_UP:
        panel->selected = 1;
        break;
    case KEY_VOLUME_DOWN:
        panel->selected = 0;
        break;
    case KEY_PLAY:
        if (panel->selected == 0) {
            PlaybackMode mode = player_playback_mode(panel->settings->player);
            mode = (PlaybackMode)((mode + 1) % 3);
            settings_playback_set_mode(panel->settings, mode, hal_clock_microseconds());
        } else {
            int sleep = settings_sleep_current(panel->settings);
            sleep = (sleep + 1) % SLEEP_TIMER_OPTION_COUNT;
            settings_sleep_apply(panel->settings, sleep, now_ms);
        }
        break;
    case KEY_BACK:
        return screen_action_pop();
    default:
        break;
    }
    return screen_action_none();
}

static void enter(void *context, uint32_t now_ms)
{
    PlaybackPanelScreen *panel = context;
    panel->selected = 0;
    panel->now_ms = now_ms;
}

static void update(void *context, uint32_t now_ms)
{
    PlaybackPanelScreen *panel = context;
    panel->now_ms = now_ms;
}

Screen *playback_panel_screen_init(PlaybackPanelScreen *panel, SettingsContext *settings,
                                   const Theme *theme)
{
    *panel = (PlaybackPanelScreen){
        .settings = settings, .theme = theme, .selected = 0, .now_ms = 0};
    panel->screen = (Screen){.title = "Playback",
                             .context = panel,
                             .full_screen = true,
                             .paints_every_pixel = true,
                             .select_on_press = true,
                             .enter = enter,
                             .handle_key = handle_key,
                             .update = update,
                             .draw = draw};
    return &panel->screen;
}

#else

typedef int PlaybackPanelTranslationUnit;

#endif
