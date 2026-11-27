#include "default_theme.h"

#include "core/resident.h"
#include "fonts/fonts.h"
#include "hal/display.h"
#include "settings/settings.h"

#ifdef ECHO_LITE
ECHO_RESIDENT const ThemeColors DEFAULT_THEME_DARK_COLORS = {
    .background = COLOR_RGB(250, 250, 249),
    .text = COLOR_RGB(10, 10, 10),
    .text_secondary = COLOR_RGB(75, 85, 99),
    .text_tertiary = COLOR_RGB(107, 114, 128),
    .selection_background = COLOR_RGB(0, 111, 238),
    .selection_top = COLOR_RGB(0, 111, 238),
    .selection_shine = COLOR_RGB(0, 111, 238),
    .selection_text = COLOR_RGB(255, 255, 255),
    .header_background = COLOR_RGB(255, 255, 255),
    .header_top = COLOR_RGB(255, 255, 255),
    .header_text = COLOR_RGB(10, 10, 10),
    .divider = COLOR_RGB(209, 213, 219),
    .scrollbar_thumb = COLOR_RGB(156, 163, 175),
    .danger = COLOR_RGB(239, 68, 68),
    .success = COLOR_RGB(34, 197, 94),
    .chevron = COLOR_RGB(10, 10, 10),
};
ECHO_RESIDENT const ThemeColors LITE_DARK_COLORS = {
    .background = COLOR_RGB(11, 11, 13),
    .text = COLOR_RGB(244, 244, 245),
    .text_secondary = COLOR_RGB(161, 161, 170),
    .text_tertiary = COLOR_RGB(113, 113, 122),
    .selection_background = COLOR_RGB(0, 111, 238),
    .selection_top = COLOR_RGB(0, 111, 238),
    .selection_shine = COLOR_RGB(0, 111, 238),
    .selection_text = COLOR_RGB(255, 255, 255),
    .header_background = COLOR_RGB(23, 23, 28),
    .header_top = COLOR_RGB(31, 31, 38),
    .header_text = COLOR_RGB(244, 244, 245),
    .divider = COLOR_RGB(63, 63, 70),
    .scrollbar_thumb = COLOR_RGB(113, 113, 122),
    .danger = COLOR_RGB(239, 68, 68),
    .success = COLOR_RGB(34, 197, 94),
    .chevron = COLOR_RGB(161, 161, 170),
};

#else
ECHO_RESIDENT const ThemeColors DEFAULT_THEME_DARK_COLORS = {
    .background = COLOR_RGB(16, 17, 20),
    .text = COLOR_RGB(242, 242, 245),
    .text_secondary = COLOR_RGB(142, 142, 150),
    .text_tertiary = COLOR_RGB(98, 98, 106),
    .selection_background = COLOR_RGB(34, 96, 208),
    .selection_top = COLOR_RGB(82, 145, 240),
    .selection_shine = COLOR_RGB(120, 170, 248),
    .selection_text = COLOR_RGB(255, 255, 255),
    .header_background = COLOR_RGB(22, 23, 27),
    .header_top = COLOR_RGB(46, 47, 54),
    .header_text = COLOR_RGB(242, 242, 245),
    .divider = COLOR_RGB(48, 49, 56),
    .scrollbar_thumb = COLOR_RGB(96, 97, 106),
    .danger = COLOR_RGB(235, 77, 61),
    .success = COLOR_RGB(52, 199, 89),
    .chevron = COLOR_RGB(110, 110, 118),
};
#endif

ECHO_RESIDENT const Theme DEFAULT_THEME = {
    .colors = &DEFAULT_THEME_DARK_COLORS,
    .boxes = &THEME_STANDARD_BOXES,
    .fonts =
        {
            .title = &FONT_INTER_SEMIBOLD_16,
            .body = &FONT_INTER_REGULAR_16,
            .small = &FONT_INTER_REGULAR_12,
        },
    .metrics =
        {
            .screen_width = HAL_DISPLAY_WIDTH,
            .screen_height = HAL_DISPLAY_HEIGHT,
#ifdef ECHO_LITE
            .header_height = 0,
            .header_inset = 12,
            .row_height = 26,
            .padding = 12,
            .scrollbar_width = 2,
            .scrollbar_inset = 2,
            .detail_gap = 8,
            .status_icon_size = 10,
#else
            .header_height = 24,
            .header_inset = 72,
            .row_height = 24,
            .padding = 10,
            .scrollbar_width = 3,
            .scrollbar_inset = 2,
            .detail_gap = 8,
            .status_icon_size = 10,
#endif
        },
    .marquee = {.pause_ms = 1200, .speed_px_per_s = 40, .gap_px = 32},
};

enum {
    PORTRAIT_STATUS_HEIGHT = 16,
    PORTRAIT_TITLE_HEIGHT = 24,
    LARGE_STATUS_HEIGHT = 22,
    LARGE_STATUS_ICON_SIZE = 14
};

#ifdef ECHO_LITE
void lite_theme_force(Theme *theme, bool dark)
{
    theme->colors = dark ? &LITE_DARK_COLORS : &DEFAULT_THEME_DARK_COLORS;
    theme->boxes = &THEME_STANDARD_BOXES;
    theme->icons = NULL;
    theme->wallpaper = NULL;
    theme->layout = NULL;
    theme->fonts = DEFAULT_THEME.fonts;
    theme->marquee = DEFAULT_THEME.marquee;
    theme->selection = THEME_SELECTION_FLAT;
    theme->no_menu_panel = true;
    theme->metrics.header_height = 0;
    theme->metrics.header_inset = 12;
    theme->metrics.row_height = theme_is_portrait(theme) ? 28 : 26;
    theme->metrics.padding = 12;
    theme->metrics.scrollbar_width = 2;
    theme->metrics.scrollbar_inset = 2;
    theme->metrics.status_height = 0;
}
#endif

Theme default_theme_for(bool portrait)
{
    Theme theme = DEFAULT_THEME;
    if (portrait) {
        theme.metrics.screen_width = HAL_DISPLAY_SHORT_SIDE;
        theme.metrics.screen_height = HAL_DISPLAY_LONG_SIDE;
        theme.metrics.status_height = PORTRAIT_STATUS_HEIGHT;
        theme.metrics.header_height = PORTRAIT_STATUS_HEIGHT + PORTRAIT_TITLE_HEIGHT;
        theme.metrics.header_inset = theme.metrics.padding;
    }
    return theme;
}

void default_theme_enlarge_status(Theme *theme)
{
    if (!theme_is_portrait(theme)) {
        return;
    }
    theme->metrics.status_height = LARGE_STATUS_HEIGHT;
    theme->metrics.header_height = LARGE_STATUS_HEIGHT + PORTRAIT_TITLE_HEIGHT;
    theme->metrics.status_icon_size = LARGE_STATUS_ICON_SIZE;
    theme->fonts.status = &FONT_INTER_REGULAR_16;
}

bool default_theme_takes_accent(const ThemePackage *package)
{
    return package != NULL && package->accent_count > 0;
}

bool default_theme_takes_highlight(const ThemePackage *package)
{
    return package == NULL || package->selection == THEME_SELECTION_GRADIENT;
}

int default_theme_accent(const ThemePackage *package, int accent)
{
    return package != NULL && accent >= 0 && accent < package->accent_count ? accent : 0;
}

static void flatten(Theme *theme, ThemeChoice choice)
{
    if (choice.flat && theme->selection == THEME_SELECTION_GRADIENT) {
        theme->selection = THEME_SELECTION_FLAT;
    }
}

void default_theme_dress(Theme *theme, const ThemePackage *package, ThemeChoice choice,
                         ThemeColors *tinted)
{
    if (package != NULL) {
        theme->colors = &package->colors;
        theme->boxes = &package->boxes;
        theme->selection = (ThemeSelection)package->selection;
        theme->no_menu_panel = (package->flags & THEME_FLAG_NO_HOME_ART) != 0;
    }
    if (tinted != NULL && default_theme_takes_accent(package)) {
        const ThemeAccent *accent = &package->accents[default_theme_accent(package, choice.accent)];
        *tinted = package->colors;
        tinted->selection_background = accent->background;
        tinted->selection_top = choice.flat ? accent->background : accent->top;
        tinted->selection_shine = accent->shine;
        theme->colors = tinted;
    }
    flatten(theme, choice);
}

void default_theme_stage(Theme *stage, const ThemePackage *package, ThemeChoice choice,
                         ThemeColors *tinted)
{
    if (package != NULL && (package->flags & THEME_FLAG_DARK_COVER_FLOW) != 0) {
        stage->colors = &DEFAULT_THEME_DARK_COLORS;
        stage->boxes = &THEME_STANDARD_BOXES;
        stage->selection = THEME_SELECTION_GRADIENT;
        flatten(stage, choice);
        return;
    }
    default_theme_dress(stage, package, choice, tinted);
}
