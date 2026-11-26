#include "playback_panel_screen.h"

#ifdef ECHO_LITE

#include <stddef.h>

#include "hal/clock.h"
#include "player.h"
#include "settings_screens.h"
#include "ui/label.h"
#include "ui/lite_badge.h"
#include "ui/lite_icons.h"

enum {
    CARD_X = 12,
    CARD_Y = 30,
    CARD_WIDTH = 140,
    CARD_HEIGHT = 112,
    CARD_GAP = 16,
    CARD_SHADOW = 5,
    CARD_BORDER = 2,
    BADGE_CENTER_Y = 86,
    BADGE_RADIUS = 32,
    MODE_GLYPH = 38,
    MOON_GLYPH = 30,
    MOON_TOP = 64,
    SLEEP_PILL_TOP = 100,
    SLEEP_PILL_HEIGHT = 16,
    SLEEP_PILL_PAD = 10
};

static const Color MODE_BASE = COLOR_RGB(47, 168, 255);
static const Color MODE_LIGHT = COLOR_RGB(139, 215, 255);
static const Color MODE_DARK = COLOR_RGB(22, 119, 200);
static const Color SLEEP_BASE = COLOR_RGB(99, 102, 241);
static const Color SLEEP_LIGHT = COLOR_RGB(165, 180, 252);
static const Color SLEEP_DARK = COLOR_RGB(55, 48, 163);

static const char *const SLEEP_VALUES[SLEEP_TIMER_OPTION_COUNT] = {
    "", "15", "30", "45", "60", "90", "120"};

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

static void draw_card(Canvas *canvas, const Theme *theme, Rect area, Color border)
{
    Color ink = theme->colors->text;
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

static void draw_mode_badge(Canvas *canvas, const Theme *theme, Rect card, PlaybackMode mode)
{
    Point center = {card.x + card.width / 2, BADGE_CENTER_Y};
    LiteBadgeStyle style = {
        .base = MODE_BASE,
        .light = MODE_LIGHT,
        .dark = MODE_DARK,
        .ink = theme->colors->text,
        .shadow = theme->colors->text,
    };
    lite_badge_draw_circle(canvas, center, BADGE_RADIUS, &style);
    lite_icon_draw_scaled(
        canvas, mode_icon(mode),
        (Rect){center.x - MODE_GLYPH / 2, center.y - MODE_GLYPH / 2, MODE_GLYPH, MODE_GLYPH},
        COLOR_RGB(255, 255, 255));
}

static void draw_sleep_badge(Canvas *canvas, const Theme *theme, Rect card, int sleep)
{
    Point center = {card.x + card.width / 2, BADGE_CENTER_Y};
    LiteBadgeStyle style = {
        .base = SLEEP_BASE,
        .light = SLEEP_LIGHT,
        .dark = SLEEP_DARK,
        .ink = theme->colors->text,
        .shadow = theme->colors->text,
    };
    lite_badge_draw_circle(canvas, center, BADGE_RADIUS, &style);
    lite_icon_draw_scaled(
        canvas, &LITE_ICON_MOON,
        (Rect){center.x - MOON_GLYPH / 2, MOON_TOP, MOON_GLYPH, MOON_GLYPH},
        COLOR_RGB(255, 255, 255));

    TextStyle text = {.font = theme->fonts.small, .color = COLOR_RGB(255, 255, 255)};
    const char *value = sleep >= 0 && sleep < SLEEP_TIMER_OPTION_COUNT ? SLEEP_VALUES[sleep] : "";
    int width = value[0] == '\0' ? 20 : text_width(text.font, value) + SLEEP_PILL_PAD;
    Rect pill = {center.x - width / 2, SLEEP_PILL_TOP, width, SLEEP_PILL_HEIGHT};
    canvas_fill_rect(canvas, pill, SLEEP_DARK);
    if (value[0] == '\0') {
        canvas_fill_rect(canvas,
                         (Rect){center.x - 5, SLEEP_PILL_TOP + SLEEP_PILL_HEIGHT / 2 - 1, 10, 2},
                         COLOR_RGB(255, 255, 255));
    } else {
        label_draw(canvas, text,
                   (Label){.area = label_centered_area(text.font, pill, value), .text = value});
    }
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
    draw_card(canvas, theme, first,
              panel->selected == 0 ? theme->colors->selection_background : theme->colors->text);
    draw_card(canvas, theme, second,
              panel->selected == 1 ? SLEEP_BASE : theme->colors->text);

    draw_mode_badge(canvas, theme, first, mode);
    draw_sleep_badge(canvas, theme, second, sleep);
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
