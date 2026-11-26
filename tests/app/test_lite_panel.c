#include <stdint.h>
#include <string.h>

#include "app/app.h"
#include "check.h"
#include "fake_hal.h"
#include "fixture_library.h"
#include "hal/system.h"

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

static void hold(Key key)
{
    fake_hal_queue_transition(key, true, now_ms);
    now_ms += 1000;
    step();
    fake_hal_queue_transition(key, false, now_ms);
    now_ms += 10;
    step();
}

static bool title_is(const char *expected)
{
    return strcmp(expected, app_screen_title()) == 0;
}

static void move_down(int times)
{
    for (int count = 0; count < times; count++) {
        tap(KEY_VOLUME_UP);
    }
}

static void lite_animation_arena_is_available(void)
{
    uint32_t size = 0;
    void *memory = hal_system_lite_animation(&size);
    CHECK(memory != NULL);
    CHECK_EQUAL(HAL_LITE_ANIMATION_BYTES, size);
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
        TEST_CASE(lite_animation_arena_is_available),
        TEST_CASE(lite_settings_still_opens_after_diagnostics_removed),
        TEST_CASE(lite_settings_groups_have_only_visible_rows),
        TEST_CASE(lite_settings_rows_are_compact),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
