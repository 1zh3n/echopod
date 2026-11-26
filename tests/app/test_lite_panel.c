#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app/app.h"
#include "check.h"
#include "fake_hal.h"
#include "fixture_library.h"
#include "default_theme.h"
#include "gfx/canvas.h"
#include "gfx/color.h"
#include "hal/system.h"
#include "lite_animation.h"
#include "screens/charge_screen.h"
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

static uint8_t animation_blob[300000];

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

static void boot_screen_draws_only_the_animation(void)
{
    start_app_with_library();
    CHECK(load_animation_asset());
    step();
    CHECK(fake_hal_pixel(160, 85) != COLOR_RGB(250, 250, 249));
}

static void boot_screen_is_static(void)
{
    start_app_with_library();
    CHECK(load_animation_asset());
    step();
    Color first = fake_hal_pixel(160, 85);
    now_ms += 500;
    step();
    CHECK(fake_hal_pixel(160, 85) == first);
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

static void lite_settings_still_opens_after_diagnostics_removed(void)
{
    start_app_with_library();
    move_down(2);
    tap(KEY_PLAY);
    CHECK(title_is("Settings"));
    tap(KEY_PLAY);
    CHECK(title_is("Sound"));
    tap(KEY_BACK);
    CHECK(title_is("Settings"));
}

static void lite_settings_groups_have_only_visible_rows(void)
{
    start_app_with_library();
    move_down(2);
    tap(KEY_PLAY);
    CHECK(title_is("Settings"));
    tap(KEY_PLAY);
    CHECK(title_is("Sound"));
    tap(KEY_BACK);
    move_down(1);
    tap(KEY_PLAY);
    CHECK(title_is("Display"));
    tap(KEY_BACK);
    move_down(1);
    tap(KEY_PLAY);
    CHECK(title_is("Library"));
}

static void lite_settings_rows_are_compact(void)
{
    start_app_with_library();
    move_down(2);
    tap(KEY_PLAY);
    tap(KEY_PLAY);
    move_down(5);
    tap(KEY_PLAY);
    CHECK(title_is("Equalizer"));
    tap(KEY_BACK);
    tap(KEY_BACK);
    move_down(1);
    tap(KEY_PLAY);
    move_down(2);
    tap(KEY_PLAY);
    CHECK(title_is("Animations"));
    tap(KEY_BACK);
    tap(KEY_BACK);
    move_down(1);
    tap(KEY_PLAY);
    move_down(2);
    tap(KEY_PLAY);
    CHECK(title_is("Lyrics"));
}

static void open_all_songs_and_play(void)
{
    tap(KEY_PLAY);
    CHECK(title_is("All Songs"));
    tap(KEY_PLAY);
    CHECK(title_is("Now Playing"));
}

static void short_menu_on_the_panel_returns_to_now_playing(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    int depth = app_screen_depth();
    hold(KEY_MENU);
    CHECK(title_is("Playback"));
    CHECK_EQUAL(depth + 1, app_screen_depth());
    tap(KEY_MENU);
    CHECK(title_is("Now Playing"));
    CHECK_EQUAL(depth, app_screen_depth());
    hold(KEY_MENU);
    CHECK(title_is("Playback"));
    tap(KEY_MENU);
    CHECK(title_is("Now Playing"));
    CHECK_EQUAL(depth, app_screen_depth());
}

static void long_menu_on_the_panel_closes_it_without_opening_settings(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    int depth = app_screen_depth();
    hold(KEY_MENU);
    CHECK(title_is("Playback"));
    hold(KEY_MENU);
    CHECK(title_is("Now Playing"));
    CHECK_EQUAL(depth, app_screen_depth());
}

int main(void)
{
    TestCase cases[] = {
        TEST_CASE(short_menu_on_the_panel_returns_to_now_playing),
        TEST_CASE(long_menu_on_the_panel_closes_it_without_opening_settings),
        TEST_CASE(compact_icons_are_native_18px),
        TEST_CASE(lite_animation_arena_is_available),
        TEST_CASE(lite_settings_still_opens_after_diagnostics_removed),
        TEST_CASE(lite_settings_groups_have_only_visible_rows),
        TEST_CASE(lite_settings_rows_are_compact),
        TEST_CASE(lite_animation_streams_all_frames),
        TEST_CASE(boot_screen_draws_only_the_animation),
        TEST_CASE(boot_screen_is_static),
        TEST_CASE(colorful_home_has_no_text),
        TEST_CASE(goodbye_screen_has_no_icon),
        TEST_CASE(charging_screen_has_green_shadow_and_no_text),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
