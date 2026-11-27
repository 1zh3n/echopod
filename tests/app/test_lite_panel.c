#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app/app.h"
#include "check.h"
#include "default_theme.h"
#include "fake_hal.h"
#include "fixture_library.h"
#include "gfx/canvas.h"
#include "gfx/color.h"
#include "hal/system.h"
#include "lite_animation.h"
#include "screens/charge_screen.h"
#include "ui/lite_badge.h"
#include "ui/lite_color_icons.h"
#include "ui/lite_frame.h"
#include "ui/lite_icons.h"

enum {
    START_MS = 1000,
    KEY_HOLD_MS = 30,
};

static uint32_t now_ms;

static void step(void)
{
    fake_hal_set_milliseconds(now_ms);
    app_step();
}

static unsigned area_hash(int x, int y, int width, int height)
{
    unsigned hash = 0;
    for (int row = y; row < y + height; row++) {
        for (int column = x; column < x + width; column++) {
            hash = hash * 33 + fake_hal_pixel(column, row);
        }
    }
    return hash;
}

static bool area_has_color(int x, int y, int width, int height, Color color)
{
    for (int row = y; row < y + height; row++) {
        for (int column = x; column < x + width; column++) {
            if (fake_hal_pixel(column, row) == color) {
                return true;
            }
        }
    }
    return false;
}

static void start_app_with_library(void)
{
    uint32_t size = 0;
    uint32_t covers_size = 0;
    const uint8_t *bytes = fixture_bytes(&size);
    const uint8_t *covers = fixture_covers_bytes(&covers_size);
    fake_hal_reset();
    fake_hal_add_file("/.echo/library.db", bytes, size);
    fake_hal_add_file("/.echo/covers.bin", covers, covers_size);
    fake_hal_add_file("/.echo/build.tmp", NULL, 16 * 1024 * 1024);
    now_ms = START_MS;
    fake_hal_set_milliseconds(now_ms);
    app_start();
}

static void tap(Key key)
{
    fake_hal_queue_transition(key, true, now_ms);
    fake_hal_queue_transition(key, false, now_ms + KEY_HOLD_MS);
    now_ms += KEY_HOLD_MS + 10;
    step();
}

static void hold_for(Key key, uint32_t duration_ms)
{
    fake_hal_queue_transition(key, true, now_ms);
    now_ms += duration_ms;
    step();
    fake_hal_queue_transition(key, false, now_ms);
    now_ms += 10;
    step();
}

static void hold(Key key)
{
    hold_for(key, 1000);
}

static bool title_is(const char *expected)
{
    return strcmp(expected, app_screen_title()) == 0;
}

static void wait_ms(uint32_t duration_ms)
{
    now_ms += duration_ms;
    step();
}

static void move_down(int times)
{
    for (int count = 0; count < times; count++) {
        tap(KEY_VOLUME_UP);
    }
}

static void gear_glyph_draws_only_white_outline(void)
{
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    canvas_clear(&canvas, COLOR_RGB(1, 2, 3));
    color_icon_draw_glyph_scaled(&canvas, &LITE_COLOR_ICON_GEAR, (Rect){10, 10, 60, 60},
                                 (Rect){13, 13, 38, 38});
    CHECK(canvas_pixel_at(&canvas, 52, 40) == COLOR_RGB(255, 255, 255));
    CHECK(canvas_pixel_at(&canvas, 40, 40) == COLOR_RGB(1, 2, 3));
}

static void badge_draws_flat_circle_and_half(void)
{
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    canvas_clear(&canvas, COLOR_RGB(250, 250, 249));
    LiteBadgeStyle style = {
        .base = COLOR_RGB(47, 168, 255),
        .light = COLOR_RGB(139, 215, 255),
        .dark = COLOR_RGB(22, 119, 200),
        .ink = COLOR_RGB(10, 10, 10),
        .shadow = COLOR_RGB(10, 10, 10),
    };
    lite_badge_draw_circle(&canvas, (Point){80, 80}, 32, &style);
    CHECK(canvas_pixel_at(&canvas, 80, 80) == style.base);
    CHECK(canvas_pixel_at(&canvas, 80 - 36, 80) == COLOR_RGB(250, 250, 249));

    lite_badge_draw_half(&canvas, (Point){20, 80}, 40, &style);
    CHECK(canvas_pixel_at(&canvas, 20, 80) == style.base);
    CHECK(canvas_pixel_at(&canvas, 19, 80) == COLOR_RGB(250, 250, 249));
}

static void every_lite_icon_draws(void)
{
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    const IconMask *masks[] = {
        &LITE_ICON_SHUFFLE,          &LITE_ICON_LOOP,
        &LITE_ICON_LOOP_ONE,         &LITE_ICON_MOON,
        &LITE_ICON_SHUFFLE_18,       &LITE_ICON_LOOP_18,
        &LITE_ICON_LOOP_ONE_18,      &LITE_ICON_PLAY_18,
        &LITE_ICON_PAUSE_18,         &LITE_ICON_CATEGORY_SOUND,
        &LITE_ICON_CATEGORY_DISPLAY, &LITE_ICON_CATEGORY_LIBRARY,
        &LITE_ICON_CATEGORY_BUTTONS,
    };
    const ColorIcon *colors[] = {&LITE_COLOR_ICON_ALL, &LITE_COLOR_ICON_LIST,
                                 &LITE_COLOR_ICON_GEAR};
    for (size_t index = 0; index < sizeof masks / sizeof masks[0]; index++) {
        canvas_clear(&canvas, COLOR_RGB(1, 2, 3));
        lite_icon_draw_scaled(&canvas, masks[index], (Rect){100, 40, 48, 48},
                              COLOR_RGB(255, 255, 255));
        CHECK(area_has_color(100, 40, 48, 48, COLOR_RGB(255, 255, 255)));
    }
    for (size_t index = 0; index < sizeof colors / sizeof colors[0]; index++) {
        canvas_clear(&canvas, COLOR_RGB(1, 2, 3));
        color_icon_draw_scaled(&canvas, colors[index], (Rect){100, 40, 64, 64});
        CHECK(area_has_color(100, 40, 64, 64, COLOR_RGB(47, 168, 255)) ||
              area_has_color(100, 40, 64, 64, COLOR_RGB(255, 105, 180)));
    }
}

static void category_icons_are_40px(void)
{
    CHECK_EQUAL(40, LITE_ICON_CATEGORY_SOUND.width);
    CHECK_EQUAL(40, LITE_ICON_CATEGORY_DISPLAY.height);
    CHECK_EQUAL(40, LITE_ICON_CATEGORY_LIBRARY.width);
    CHECK_EQUAL(40, LITE_ICON_CATEGORY_BUTTONS.height);
}

static void compact_icons_are_native_18px(void)
{
    CHECK_EQUAL(18, LITE_ICON_SHUFFLE_18.width);
    CHECK_EQUAL(18, LITE_ICON_LOOP_18.height);
    CHECK_EQUAL(18, LITE_ICON_LOOP_ONE_18.width);
    CHECK_EQUAL(18, LITE_ICON_PLAY_18.height);
    CHECK_EQUAL(18, LITE_ICON_PAUSE_18.width);
}

static void lite_animation_arena_is_available(void)
{
    uint32_t size = 0;
    void *memory = hal_system_lite_animation(&size);
    CHECK(memory != NULL);
    CHECK_EQUAL(HAL_LITE_ANIMATION_BYTES, size);
}

static uint8_t animation_blob[400000];
static uint8_t icons_blob[20000];

static bool load_icon_asset(void)
{
    FILE *file = fopen(ECHO_LITE_ICONS_ASSET, "rb");
    if (file == NULL) {
        return false;
    }
    size_t size = fread(icons_blob, 1, sizeof icons_blob, file);
    fclose(file);
    fake_hal_set_lite_icons(icons_blob, (uint32_t)size);
    return size > 0;
}

static bool load_animation_asset(void)
{
    FILE *file = fopen(ECHO_LITE_ANIMATION_ASSET, "rb");
    if (file == NULL) {
        return false;
    }
    size_t size = fread(animation_blob, 1, sizeof animation_blob, file);
    fclose(file);
    fake_hal_set_lite_animation(animation_blob, (uint32_t)size);
    return size > 0;
}

static void lite_animation_streams_all_frames(void)
{
    CHECK(load_animation_asset());
    lite_animation_reset();
    CHECK(lite_animation_load());
    CHECK(lite_animation_ready());
    CHECK_EQUAL(96, lite_animation_frames());
}

static bool draw_boot_image(Canvas *canvas)
{
    canvas_clear(canvas, COLOR_RGB(250, 250, 249));
    return lite_boot_draw(canvas, COLOR_RGB(10, 10, 10), false);
}

static void boot_screen_has_large_frame(void)
{
    fake_hal_reset();
    CHECK(load_animation_asset());
    lite_animation_reset();
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    CHECK(draw_boot_image(&canvas));
    CHECK(canvas_pixel_at(&canvas, 1, 1) == COLOR_RGB(10, 10, 10));
    CHECK(canvas_pixel_at(&canvas, 3, 3) != COLOR_RGB(250, 250, 249));
}

static void boot_screen_draws_only_the_animation(void)
{
    fake_hal_reset();
    CHECK(load_animation_asset());
    lite_animation_reset();
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    CHECK(draw_boot_image(&canvas));
    CHECK(canvas_pixel_at(&canvas, 160, 85) != COLOR_RGB(250, 250, 249));
}

static void boot_screen_is_static(void)
{
    fake_hal_reset();
    CHECK(load_animation_asset());
    lite_animation_reset();
    static Color first[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    static Color second[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas one = canvas_make(first, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    Canvas two = canvas_make(second, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    CHECK(draw_boot_image(&one));
    CHECK(draw_boot_image(&two));
    CHECK(memcmp(first, second, sizeof first) == 0);
}

static void colorful_home_has_no_text(void)
{
    start_app_with_library();
    wait_ms(1400);
    CHECK(fake_hal_pixel(160, 95) == COLOR_RGB(47, 168, 255));
    CHECK(fake_hal_pixel(160, 118) == COLOR_RGB(255, 255, 255));
}

static void goodbye_screen_has_no_icon(void)
{
    start_app_with_library();
    hold_for(KEY_POWER, 2000);
    CHECK(fake_hal_pixel(160, 60) == COLOR_RGB(255, 255, 255));
}

static void charging_screen_has_green_shadow_and_no_text(void)
{
    CHECK(load_animation_asset());
    lite_animation_reset();
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    HalBattery battery = {
        .known = true, .percent = 64, .millivolts = 3900, .usb_power = true, .charging = true};
    charge_screen_draw(&canvas, &DEFAULT_THEME, battery, 1000);
    CHECK(pixels[142 * HAL_DISPLAY_WIDTH + 65] == DEFAULT_THEME.colors->success);
    CHECK(pixels[150 * HAL_DISPLAY_WIDTH + 160] == DEFAULT_THEME.colors->background);
    CHECK(pixels[85 * HAL_DISPLAY_WIDTH + 160] != DEFAULT_THEME.colors->header_background);
}

static void settings_starts_on_sound(void)
{
    start_app_with_library();
    move_down(2);
    tap(KEY_PLAY);
    wait_ms(300);
    CHECK(title_is("Settings"));
    CHECK(fake_hal_pixel(20, 50) == COLOR_RGB(47, 168, 255));
    CHECK(fake_hal_pixel(160, 40) == COLOR_RGB(0, 111, 238));
}

static void settings_long_press_switches_category(void)
{
    start_app_with_library();
    move_down(2);
    tap(KEY_PLAY);
    wait_ms(300);
    hold(KEY_PLAY);
    wait_ms(100);
    CHECK(title_is("Settings"));
    CHECK(fake_hal_pixel(20, 50) == COLOR_RGB(53, 199, 232));
    hold(KEY_PLAY);
    wait_ms(100);
    CHECK(fake_hal_pixel(20, 50) == COLOR_RGB(255, 159, 28));
    hold(KEY_PLAY);
    wait_ms(100);
    CHECK(fake_hal_pixel(20, 50) == COLOR_RGB(245, 158, 11));
    hold(KEY_PLAY);
    wait_ms(100);
    CHECK(fake_hal_pixel(20, 50) == COLOR_RGB(139, 92, 246));
    CHECK(fake_hal_pixel(39, 95) == COLOR_RGB(139, 92, 246));
    CHECK(fake_hal_pixel(51, 95) == COLOR_RGB(255, 255, 255));
    CHECK(fake_hal_pixel(302, 18) == COLOR_RGB(139, 92, 246));
}

static void settings_short_press_cycles_value(void)
{
    start_app_with_library();
    move_down(2);
    tap(KEY_PLAY);
    wait_ms(300);
    unsigned before = area_hash(250, 30, 46, 26);
    int depth = app_screen_depth();
    tap(KEY_PLAY);
    wait_ms(100);
    CHECK(title_is("Settings"));
    CHECK_EQUAL(depth, app_screen_depth());
    CHECK(area_hash(250, 30, 46, 26) != before);
}

static void settings_subpage_opens(void)
{
    start_app_with_library();
    move_down(2);
    tap(KEY_PLAY);
    wait_ms(300);
    move_down(5);
    tap(KEY_PLAY);
    wait_ms(100);
    CHECK(title_is("Equalizer"));
    tap(KEY_BACK);
    CHECK(title_is("Settings"));
}

static void open_all_songs_and_play(void)
{
    tap(KEY_PLAY);
    CHECK(title_is("All Songs"));
    tap(KEY_PLAY);
    CHECK(title_is("Now Playing"));
}

static void now_playing_uses_native_18px_icons(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    wait_ms(300);
    CHECK(area_has_color(262, 8, 18, 18, COLOR_RGB(0, 111, 238)));
    CHECK(area_has_color(288, 8, 18, 18, COLOR_RGB(10, 10, 10)));
}

static void quick_panel_is_icon_only(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    hold(KEY_PLAY);
    wait_ms(300);
    CHECK(title_is("Playback"));
    CHECK(fake_hal_pixel(55, 86) == COLOR_RGB(47, 168, 255));
    CHECK(fake_hal_pixel(211, 86) == COLOR_RGB(99, 102, 241));
    CHECK(fake_hal_pixel(229, 104) == COLOR_RGB(55, 48, 163));
    CHECK(fake_hal_pixel(160, 155) == COLOR_RGB(250, 250, 249));
}

static void quick_panel_sleep_value_fits(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    hold(KEY_PLAY);
    wait_ms(300);
    tap(KEY_VOLUME_UP);
    tap(KEY_PLAY);
    wait_ms(100);
    CHECK(area_has_color(224, 100, 28, 16, COLOR_RGB(255, 255, 255)));
    CHECK(fake_hal_pixel(168, 31) == COLOR_RGB(99, 102, 241));
    for (int step = 0; step < 5; step++) {
        tap(KEY_PLAY);
        wait_ms(20);
    }
    wait_ms(100);
    CHECK(area_has_color(220, 100, 36, 16, COLOR_RGB(255, 255, 255)));
    CHECK(fake_hal_pixel(270, 108) != COLOR_RGB(55, 48, 163));
}

static void short_menu_on_the_panel_returns_to_now_playing(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    int depth = app_screen_depth();
    hold(KEY_PLAY);
    CHECK(title_is("Playback"));
    CHECK_EQUAL(depth + 1, app_screen_depth());
    tap(KEY_MENU);
    CHECK(title_is("Now Playing"));
    CHECK_EQUAL(depth, app_screen_depth());
    hold(KEY_PLAY);
    CHECK(title_is("Playback"));
    tap(KEY_MENU);
    CHECK(title_is("Now Playing"));
    CHECK_EQUAL(depth, app_screen_depth());
}

static void long_menu_on_now_playing_opens_settings(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    hold(KEY_MENU);
    CHECK(title_is("Settings"));
    tap(KEY_BACK);
    CHECK(title_is("Now Playing"));
}

static void long_back_on_panel_goes_home(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    hold(KEY_PLAY);
    CHECK(title_is("Playback"));
    hold(KEY_BACK);
    CHECK(title_is("EchoPod"));
}

static void long_menu_on_the_panel_opens_settings(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    hold(KEY_PLAY);
    CHECK(title_is("Playback"));
    hold(KEY_MENU);
    CHECK(title_is("Settings"));
    tap(KEY_BACK);
    CHECK(title_is("Playback"));
    tap(KEY_MENU);
    CHECK(title_is("Now Playing"));
}

static void open_settings_display(void)
{
    start_app_with_library();
    move_down(2);
    tap(KEY_PLAY);
    wait_ms(300);
    hold(KEY_PLAY);
    wait_ms(100);
}

static void switch_orientation_to_portrait(void)
{
    open_settings_display();
    move_down(1);
    tap(KEY_PLAY);
    wait_ms(300);
    tap(KEY_BACK);
    wait_ms(300);
}

static void switch_theme_to_dark(void)
{
    open_settings_display();
    tap(KEY_PLAY);
    wait_ms(300);
    tap(KEY_BACK);
    wait_ms(300);
}

static void dark_palette_and_borders(void)
{
    Theme dark = default_theme_for(false);
    lite_theme_force(&dark, true);
    CHECK(dark.colors->background == COLOR_RGB(11, 11, 13));
    CHECK(dark.colors->header_background == COLOR_RGB(23, 23, 28));
    CHECK(lite_theme_is_dark(&dark));
    CHECK(lite_card_border_color(&dark) == COLOR_RGB(82, 82, 91));
    CHECK_EQUAL(1, lite_card_border_width(&dark));
    CHECK(lite_card_shadow_color(&dark) == COLOR_RGB(82, 82, 91));
    CHECK_EQUAL(5, lite_list_rows(&dark));

    Theme light = default_theme_for(false);
    lite_theme_force(&light, false);
    CHECK(light.colors->background == COLOR_RGB(250, 250, 249));
    CHECK(!lite_theme_is_dark(&light));
    CHECK_EQUAL(2, lite_card_border_width(&light));
    CHECK(lite_card_border_color(&light) == light.colors->text);

    Theme portrait = default_theme_for(true);
    lite_theme_force(&portrait, true);
    CHECK_EQUAL(170, portrait.metrics.screen_width);
    CHECK_EQUAL(320, portrait.metrics.screen_height);
    CHECK_EQUAL(28, portrait.metrics.row_height);
    CHECK_EQUAL(9, lite_list_rows(&portrait));
}

static void settings_can_switch_to_portrait(void)
{
    switch_orientation_to_portrait();
    CHECK(fake_hal_pixel(85, 164) == COLOR_RGB(139, 92, 246));
    CHECK(fake_hal_pixel(160, 76) != COLOR_RGB(139, 92, 246));

    tap(KEY_PLAY);
    wait_ms(300);
    CHECK(fake_hal_pixel(20, 50) == COLOR_RGB(250, 250, 249));
    CHECK(fake_hal_pixel(12, 34) == COLOR_RGB(53, 199, 232));
    CHECK(fake_hal_pixel(20, 100) == COLOR_RGB(0, 111, 238));
}

static void settings_can_switch_to_dark(void)
{
    switch_theme_to_dark();
    CHECK(fake_hal_pixel(10, 10) == COLOR_RGB(11, 11, 13));
    CHECK(fake_hal_pixel(120, 30) == COLOR_RGB(23, 23, 28));
    CHECK(fake_hal_pixel(108, 24) == COLOR_RGB(82, 82, 91));
}

static void switch_dark_portrait(void)
{
    open_settings_display();
    tap(KEY_PLAY);
    wait_ms(300);
    move_down(1);
    tap(KEY_PLAY);
    wait_ms(300);
    tap(KEY_BACK);
    wait_ms(300);
}

static void dark_portrait_home_and_settings(void)
{
    switch_dark_portrait();
    CHECK(fake_hal_pixel(10, 10) == COLOR_RGB(11, 11, 13));
    CHECK(fake_hal_pixel(85, 110) == COLOR_RGB(23, 23, 28));
    CHECK(fake_hal_pixel(17, 96) == COLOR_RGB(82, 82, 91));
    CHECK(fake_hal_pixel(85, 164) == COLOR_RGB(139, 92, 246));

    tap(KEY_PLAY);
    wait_ms(300);
    CHECK(fake_hal_pixel(20, 50) == COLOR_RGB(11, 11, 13));
    CHECK(fake_hal_pixel(12, 34) == COLOR_RGB(53, 199, 232));
    CHECK(fake_hal_pixel(20, 100) == COLOR_RGB(0, 111, 238));
}

static void portrait_now_playing_and_panel(void)
{
    switch_orientation_to_portrait();
    move_down(2);
    open_all_songs_and_play();
    wait_ms(300);
    CHECK(area_has_color(122, 8, 18, 18, COLOR_RGB(0, 111, 238)));
    CHECK(area_has_color(147, 8, 18, 18, COLOR_RGB(10, 10, 10)));

    hold(KEY_PLAY);
    wait_ms(300);
    CHECK(fake_hal_pixel(55, 91) == COLOR_RGB(47, 168, 255));
    CHECK(fake_hal_pixel(55, 223) == COLOR_RGB(99, 102, 241));
    CHECK(fake_hal_pixel(12, 35) == COLOR_RGB(0, 111, 238));
}

static void boot_has_portrait_still(void)
{
    CHECK(load_animation_asset());
    lite_animation_reset();
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_LONG_SIDE];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, 170, 320});
    canvas_clear(&canvas, COLOR_RGB(250, 250, 249));
    CHECK(lite_boot_draw(&canvas, COLOR_RGB(10, 10, 10), true));
    CHECK(canvas_pixel_at(&canvas, 1, 1) == COLOR_RGB(10, 10, 10));
    CHECK(canvas_pixel_at(&canvas, 3, 3) != COLOR_RGB(250, 250, 249));
}

static void charge_card_is_portrait(void)
{
    CHECK(load_animation_asset());
    lite_animation_reset();
    Theme theme = default_theme_for(true);
    lite_theme_force(&theme, false);
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_LONG_SIDE];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, 170, 320});
    HalBattery battery = {
        .known = true, .percent = 64, .millivolts = 3900, .usb_power = true, .charging = true};
    charge_screen_draw(&canvas, &theme, battery, 1000);
    CHECK(canvas_pixel_at(&canvas, 20, 110) == theme.colors->header_background);
    CHECK(canvas_pixel_at(&canvas, 160, 110) == theme.colors->success);
}

int main(void)
{
    if (!load_icon_asset()) {
        fprintf(stderr, "cannot read %s\n", ECHO_LITE_ICONS_ASSET);
        return 1;
    }
    TestCase cases[] = {
        TEST_CASE(now_playing_uses_native_18px_icons),
        TEST_CASE(quick_panel_is_icon_only),
        TEST_CASE(quick_panel_sleep_value_fits),
        TEST_CASE(short_menu_on_the_panel_returns_to_now_playing),
        TEST_CASE(long_menu_on_now_playing_opens_settings),
        TEST_CASE(long_back_on_panel_goes_home),
        TEST_CASE(long_menu_on_the_panel_opens_settings),
        TEST_CASE(gear_glyph_draws_only_white_outline),
        TEST_CASE(every_lite_icon_draws),
        TEST_CASE(badge_draws_flat_circle_and_half),
        TEST_CASE(dark_palette_and_borders),
        TEST_CASE(settings_can_switch_to_portrait),
        TEST_CASE(settings_can_switch_to_dark),
        TEST_CASE(dark_portrait_home_and_settings),
        TEST_CASE(portrait_now_playing_and_panel),
        TEST_CASE(boot_has_portrait_still),
        TEST_CASE(charge_card_is_portrait),
        TEST_CASE(category_icons_are_40px),
        TEST_CASE(compact_icons_are_native_18px),
        TEST_CASE(settings_starts_on_sound),
        TEST_CASE(settings_long_press_switches_category),
        TEST_CASE(settings_short_press_cycles_value),
        TEST_CASE(settings_subpage_opens),
        TEST_CASE(lite_animation_arena_is_available),
        TEST_CASE(lite_animation_streams_all_frames),
        TEST_CASE(boot_screen_has_large_frame),
        TEST_CASE(boot_screen_draws_only_the_animation),
        TEST_CASE(boot_screen_is_static),
        TEST_CASE(colorful_home_has_no_text),
        TEST_CASE(goodbye_screen_has_no_icon),
        TEST_CASE(charging_screen_has_green_shadow_and_no_text),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
