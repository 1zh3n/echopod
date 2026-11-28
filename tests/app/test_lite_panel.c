#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app/app.h"
#include "browse_screen.h"
#include "check.h"
#include "core/format.h"
#include "core/i18n.h"
#include "default_theme.h"
#include "fake_hal.h"
#include "fixture_library.h"
#include "gfx/canvas.h"
#include "gfx/color.h"
#include "hal/system.h"
#include "library/pinyin.h"
#include "library/sort_key.h"
#include "big_library.h"
#include "favorites_store.h"
#include "lite_animation.h"
#include "player.h"
#include "screens/charge_screen.h"
#include "favorite_track_source.h"
#include "screens/track_list_screen.h"
#include "screens/now_playing_screen.h"
#include "settings/settings.h"
#include "settings_store.h"
#include "ui/lite_badge.h"
#include "ui/lite_color_icons.h"
#include "ui/lite_frame.h"
#include "ui/lite_icons.h"

enum {
    START_MS = 1000,
    KEY_HOLD_MS = 30,
    FIFTEEN_MINUTES_MS = 15 * 60 * 1000,
    GOODBYE_MS = 1000,
};

static uint32_t now_ms;

static bool load_icon_asset(void);
static bool title_is(const char *expected);

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

static void start_app_with_start_screen(StartScreen start)
{
    uint32_t size = 0;
    uint32_t covers_size = 0;
    const uint8_t *bytes = fixture_bytes(&size);
    const uint8_t *covers = fixture_covers_bytes(&covers_size);
    Settings settings = settings_defaults();
    settings.start_screen = start;
    uint8_t encoded[SETTINGS_FILE_SIZE];
    settings_encode(&settings, encoded);
    fake_hal_reset();
    fake_hal_add_file("/.echo/library.db", bytes, size);
    fake_hal_add_file("/.echo/covers.bin", covers, covers_size);
    fake_hal_add_file(SETTINGS_STORE_PATH, encoded, sizeof encoded);
    fake_hal_add_file("/.echo/build.tmp", NULL, 16 * 1024 * 1024);
    now_ms = START_MS;
    fake_hal_set_milliseconds(now_ms);
    app_start();
}

static void start_app_with_library(void)
{
    start_app_with_start_screen(START_MAIN_MENU);
}

static void boot_starts_on_all_songs(void)
{
    start_app_with_library();
    CHECK(title_is("All Songs"));
}

static void a_saved_start_screen_still_boots_all_songs(void)
{
    start_app_with_start_screen(START_ALBUMS);
    CHECK(title_is("All Songs"));
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

static void category_dots_have_no_cardinal_spikes(void)
{
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    canvas_clear(&canvas, COLOR_RGB(250, 250, 249));
    Color blue = COLOR_RGB(47, 168, 255);
    lite_shape_fill_circle(&canvas, (Point){20, 20}, 3, blue);
    CHECK(canvas_pixel_at(&canvas, 19, 17) == blue);
    CHECK(canvas_pixel_at(&canvas, 20, 17) == blue);
    CHECK(canvas_pixel_at(&canvas, 21, 17) == blue);
    CHECK(canvas_pixel_at(&canvas, 18, 17) == COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 22, 17) == COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 17, 20) == blue);
    CHECK(canvas_pixel_at(&canvas, 23, 20) == blue);

    Color ink = COLOR_RGB(10, 10, 10);
    lite_shape_fill_circle(&canvas, (Point){40, 20}, 4, ink);
    CHECK(canvas_pixel_at(&canvas, 39, 16) == ink);
    CHECK(canvas_pixel_at(&canvas, 40, 16) == ink);
    CHECK(canvas_pixel_at(&canvas, 41, 16) == ink);
    CHECK(canvas_pixel_at(&canvas, 38, 16) == COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 42, 16) == COLOR_RGB(250, 250, 249));
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

static bool local_has_color(Canvas *canvas, int x, int y, int width, int height, Color color)
{
    for (int row = y; row < y + height; row++) {
        for (int column = x; column < x + width; column++) {
            if (canvas_pixel_at(canvas, column, row) == color) {
                return true;
            }
        }
    }
    return false;
}

static void every_lite_icon_draws(void)
{
    CHECK(load_icon_asset());
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
                                 &LITE_COLOR_ICON_GEAR, &LITE_COLOR_ICON_FAVORITE};
    for (size_t index = 0; index < sizeof masks / sizeof masks[0]; index++) {
        canvas_clear(&canvas, COLOR_RGB(1, 2, 3));
        lite_icon_draw_scaled(&canvas, masks[index], (Rect){100, 40, 48, 48},
                              COLOR_RGB(255, 255, 255));
        CHECK(local_has_color(&canvas, 100, 40, 48, 48, COLOR_RGB(255, 255, 255)));
    }
    for (size_t index = 0; index < sizeof colors / sizeof colors[0]; index++) {
        canvas_clear(&canvas, COLOR_RGB(1, 2, 3));
        color_icon_draw_scaled(&canvas, colors[index], (Rect){100, 40, 64, 64});
        CHECK(local_has_color(&canvas, 100, 40, 64, 64, COLOR_RGB(47, 168, 255)) ||
              local_has_color(&canvas, 100, 40, 64, 64, COLOR_RGB(255, 159, 28)) ||
              local_has_color(&canvas, 100, 40, 64, 64, COLOR_RGB(139, 92, 246)) ||
              local_has_color(&canvas, 100, 40, 64, 64, COLOR_RGB(236, 72, 153)));
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

static uint8_t pinyin_blob[20000];

static bool load_pinyin_asset(void)
{
    FILE *file = fopen(ECHO_PINYIN_ASSET, "rb");
    if (file == NULL) {
        return false;
    }
    size_t size = fread(pinyin_blob, 1, sizeof pinyin_blob, file);
    fclose(file);
    fake_hal_set_pinyin(pinyin_blob, (uint32_t)size);
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
    return lite_boot_draw(canvas, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT},
                          COLOR_RGB(10, 10, 10), false);
}

static void no_title_list_frame_starts_below_the_meter(void)
{
    Theme theme = default_theme_for(false);
    lite_theme_force(&theme, false);
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    canvas_clear(&canvas, COLOR_RGB(1, 2, 3));
    Rect rows = lite_list_frame_no_title(&canvas, &theme);
    CHECK_EQUAL(30, rows.y);
    CHECK(canvas_pixel_at(&canvas, 16, 8) == theme.colors->background);
    CHECK(canvas_pixel_at(&canvas, 16, 25) == theme.colors->background);
}

static void cjk_section_initials_follow_pinyin(void)
{
    CHECK(load_pinyin_asset());
    CHECK_EQUAL('z', sort_key_initial("周"));
    CHECK_EQUAL('a', sort_key_initial("あ"));
    CHECK_EQUAL((char)0x80, sort_key_initial("\xE6\xBC\xA2"));
    CHECK_EQUAL('r', sort_key_initial("日本"));
    CHECK_EQUAL('1', sort_key_initial("123"));
}

static void boot_screen_is_a_shadowed_image(void)
{
    fake_hal_reset();
    CHECK(load_animation_asset());
    lite_animation_reset();
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    CHECK(draw_boot_image(&canvas));
    CHECK(canvas_pixel_at(&canvas, 1, 1) == COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 59, 29) == COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 60, 29) != COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 259, 140) != COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 264, 145) == COLOR_RGB(10, 10, 10));
    CHECK(canvas_pixel_at(&canvas, 265, 146) == COLOR_RGB(250, 250, 249));
}

static void boot_screen_draws_the_still(void)
{
    fake_hal_reset();
    CHECK(load_animation_asset());
    lite_animation_reset();
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    CHECK(draw_boot_image(&canvas));
    CHECK(canvas_pixel_at(&canvas, 160, 85) != COLOR_RGB(250, 250, 249));
}

static void boot_draws_across_strips(void)
{
    fake_hal_reset();
    CHECK(load_animation_asset());
    lite_animation_reset();
    Rect screen = {0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT};
    Color shadow = COLOR_RGB(10, 10, 10);
    static Color pixels[HAL_DISPLAY_WIDTH * 34];
    Canvas top = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, 34});
    canvas_clear(&top, COLOR_RGB(250, 250, 249));
    CHECK(lite_boot_draw(&top, screen, shadow, false));
    CHECK(canvas_pixel_at(&top, 60, 28) == COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&top, 60, 29) != COLOR_RGB(250, 250, 249));

    Canvas middle = canvas_make(pixels, (Rect){0, 34, HAL_DISPLAY_WIDTH, 34});
    canvas_clear(&middle, COLOR_RGB(250, 250, 249));
    CHECK(lite_boot_draw(&middle, screen, shadow, false));
    CHECK(canvas_pixel_at(&middle, 60, 34) != COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&middle, 264, 34) == shadow);

    Canvas bottom = canvas_make(pixels, (Rect){0, 136, HAL_DISPLAY_WIDTH, 34});
    canvas_clear(&bottom, COLOR_RGB(250, 250, 249));
    CHECK(lite_boot_draw(&bottom, screen, shadow, false));
    CHECK(canvas_pixel_at(&bottom, 60, 140) != COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&bottom, 60, 141) == COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&bottom, 264, 141) == shadow);
    CHECK(canvas_pixel_at(&bottom, 264, 146) == COLOR_RGB(250, 250, 249));
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

static void open_all_songs_and_play(void);

static void back_at_the_root_toggles_all_songs_and_favorites(void)
{
    start_app_with_library();
    CHECK(title_is("All Songs"));
    tap(KEY_BACK);
    wait_ms(300);
    CHECK(title_is("Favorites"));
    tap(KEY_BACK);
    wait_ms(300);
    CHECK(title_is("All Songs"));
}

static void back_from_now_playing_returns_to_the_current_root(void)
{
    start_app_with_library();
    tap(KEY_PLAY);
    CHECK(title_is("Now Playing"));
    tap(KEY_BACK);
    CHECK(title_is("All Songs"));
    tap(KEY_BACK);
    wait_ms(300);
    CHECK(title_is("Favorites"));
}

static void settings_from_favorites_returns_to_favorites(void)
{
    start_app_with_library();
    tap(KEY_BACK);
    wait_ms(300);
    CHECK(title_is("Favorites"));
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("Settings"));
    tap(KEY_BACK);
    CHECK(title_is("Favorites"));
}

static void long_back_on_now_playing_opens_settings(void)
{
    start_app_with_library();
    tap(KEY_PLAY);
    CHECK(title_is("Now Playing"));
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("Settings"));
    tap(KEY_BACK);
    CHECK(title_is("All Songs"));
}

static void back_from_now_playing_returns_to_favorites_root(void)
{
    start_app_with_library();
    tap(KEY_PLAY);
    CHECK(title_is("Now Playing"));
    tap(KEY_BACK);
    tap(KEY_BACK);
    wait_ms(300);
    CHECK(title_is("Favorites"));
    tap(KEY_MENU);
    CHECK(title_is("Now Playing"));
    tap(KEY_BACK);
    CHECK(title_is("Favorites"));
}

static void long_back_on_favorites_opens_settings(void)
{
    start_app_with_library();
    tap(KEY_BACK);
    wait_ms(300);
    CHECK(title_is("Favorites"));
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("Settings"));
}

static void root_status_blocks_track_bluetooth_and_root(void)
{
    Color all_blue = COLOR_RGB(47, 168, 255);
    Color bluetooth = COLOR_RGB(15, 138, 219);
    Color pink = COLOR_RGB(236, 72, 153);
    start_app_with_library();
    wait_ms(300);
    CHECK(fake_hal_pixel(276, 8) == DEFAULT_THEME.colors->text_tertiary);
    CHECK(fake_hal_pixel(289, 8) == DEFAULT_THEME.colors->text_tertiary);
    CHECK(fake_hal_pixel(302, 8) == DEFAULT_THEME.colors->text_tertiary);
    CHECK(fake_hal_pixel(10, 10) == all_blue);
    CHECK(fake_hal_pixel(23, 10) == DEFAULT_THEME.colors->divider);
    fake_hal_bluetooth()->state = HAL_BLUETOOTH_CONNECTED;
    wait_ms(300);
    CHECK(fake_hal_pixel(276, 8) == bluetooth);
    fake_hal_bluetooth()->state = HAL_BLUETOOTH_OFF;
    wait_ms(300);
    CHECK(fake_hal_pixel(276, 8) == DEFAULT_THEME.colors->text_tertiary);
    tap(KEY_BACK);
    wait_ms(300);
    CHECK(fake_hal_pixel(10, 10) == DEFAULT_THEME.colors->divider);
    CHECK(fake_hal_pixel(23, 10) == pink);
}

static void status_blocks_draw_flat_squares(void)
{
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    canvas_clear(&canvas, COLOR_RGB(250, 250, 249));
    lite_status_block_draw(&canvas, (Point){10, 10}, COLOR_RGB(47, 168, 255));
    CHECK(canvas_pixel_at(&canvas, 10, 10) == COLOR_RGB(47, 168, 255));
    CHECK(canvas_pixel_at(&canvas, 19, 19) == COLOR_RGB(47, 168, 255));
    CHECK(canvas_pixel_at(&canvas, 20, 20) == COLOR_RGB(10, 10, 10));
}

static void status_mode_block_centers_the_one(void)
{
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_HEIGHT];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, HAL_DISPLAY_WIDTH, HAL_DISPLAY_HEIGHT});
    canvas_clear(&canvas, COLOR_RGB(250, 250, 249));
    lite_status_mode_draw(&canvas, (Point){10, 10}, COLOR_RGB(139, 92, 246), '1');
    CHECK(canvas_pixel_at(&canvas, 10, 10) == COLOR_RGB(139, 92, 246));
    CHECK(canvas_pixel_at(&canvas, 15, 12) == COLOR_RGB(255, 255, 255));
    CHECK(canvas_pixel_at(&canvas, 14, 18) == COLOR_RGB(255, 255, 255));
    CHECK(canvas_pixel_at(&canvas, 13, 12) == COLOR_RGB(139, 92, 246));
}

static void status_mode_block_cycles_gray_one_and_purple(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(200);
    tap(KEY_PLAY);
    wait_ms(200);
    tap(KEY_BACK);
    wait_ms(200);
    CHECK(fake_hal_pixel(302, 8) == DEFAULT_THEME.colors->text_tertiary);
    CHECK(fake_hal_pixel(307, 10) == COLOR_RGB(255, 255, 255));
    hold(KEY_BACK);
    wait_ms(200);
    tap(KEY_PLAY);
    wait_ms(200);
    tap(KEY_BACK);
    wait_ms(200);
    CHECK(fake_hal_pixel(302, 8) == COLOR_RGB(139, 92, 246));
    CHECK(fake_hal_pixel(307, 10) == COLOR_RGB(139, 92, 246));
}

static void now_playing_has_no_time_or_thumb(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    fake_hal_audio_set_position(30000);
    wait_ms(300);
    CHECK(!area_has_color(8, 148, 304, 14, DEFAULT_THEME.colors->text));
    CHECK(fake_hal_pixel(157, 142) == DEFAULT_THEME.colors->background);
    tap(KEY_VOLUME_UP);
    wait_ms(300);
    CHECK(area_has_color(8, 148, 304, 14, DEFAULT_THEME.colors->text));
}

static void hold_the_volume_key_seeks_without_a_hold_bar(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    fake_hal_bluetooth()->state = HAL_BLUETOOTH_CONNECTED;
    wait_ms(300);
    int seeks_before = fake_hal_audio_seek_count();
    fake_hal_queue_transition(KEY_VOLUME_UP, true, now_ms);
    now_ms += 1100;
    step();
    CHECK(fake_hal_audio_seek_count() > seeks_before);
    CHECK(fake_hal_pixel(278, 10) == COLOR_RGB(15, 138, 219));
    CHECK(fake_hal_pixel(229, 18) != COLOR_RGB(245, 158, 11));
    fake_hal_queue_transition(KEY_VOLUME_UP, false, now_ms);
    now_ms += 10;
    step();
}

static void now_playing_uses_bt_chip_with_controls(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    fake_hal_bluetooth()->state = HAL_BLUETOOTH_CONNECTED;
    wait_ms(300);
    CHECK(fake_hal_pixel(278, 10) == COLOR_RGB(15, 138, 219));
    CHECK(fake_hal_pixel(291, 10) == COLOR_RGB(245, 158, 11));
    CHECK(fake_hal_pixel(302, 10) == DEFAULT_THEME.colors->text_tertiary);
    CHECK(fake_hal_pixel(8, 6) != COLOR_RGB(0, 111, 238));
}

static void the_sleep_timer_waits_for_the_current_track(void)
{
    start_app_with_library();
    fake_hal_set_battery(
        (HalBattery){.known = true, .percent = 80, .millivolts = 3900, .usb_power = false});
    open_all_songs_and_play();
    hold(KEY_BACK);
    wait_ms(200);
    move_down(1);
    tap(KEY_PLAY);

    wait_ms(FIFTEEN_MINUTES_MS - 1000);
    wait_ms(1000);
    CHECK_EQUAL(0, fake_hal_power_off_count());

    fake_hal_audio_finish_track();
    step();
    wait_ms(GOODBYE_MS);
    CHECK_EQUAL(1, fake_hal_power_off_count());
}

static void the_sleep_quick_card_counts_down(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("Settings"));
    move_down(1);
    tap(KEY_PLAY);
    wait_ms(100);
    unsigned before = area_hash(8, 61, 304, 28);
    wait_ms(1000);
    unsigned after = area_hash(8, 61, 304, 28);
    CHECK(before != after);
}

static void date_time_uses_a_settings_card(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(200);
    move_down(4);
    tap(KEY_PLAY);
    wait_ms(200);
    move_down(1);
    tap(KEY_PLAY);
    wait_ms(300);
    CHECK(fake_hal_pixel(8, 28) == DEFAULT_THEME.colors->text);
    CHECK(fake_hal_pixel(4, 40) == DEFAULT_THEME.colors->background);
    CHECK(fake_hal_pixel(100, 35) == DEFAULT_THEME.colors->selection_background);
    CHECK(area_has_color(24, 30, 80, 19, DEFAULT_THEME.colors->selection_text));
    CHECK(fake_hal_pixel(302, 8) == DEFAULT_THEME.colors->text_tertiary);
    CHECK(area_has_color(100, 144, 120, 16, DEFAULT_THEME.colors->text_tertiary));
    tap(KEY_PLAY);
    wait_ms(300);
    CHECK(area_has_color(250, 30, 60, 19, COLOR_RGB(255, 212, 59)));
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

static FixtureLibrary favorite_fixture;
static FavoritesStore favorite_store;
static Player favorite_player;
static TrackListScreen favorite_tracks;
static FavoriteTrackSource favorite_source;
static Screen favorite_target;
static Toast favorite_toast;

static Screen *fresh_favorite_tracks(void)
{
    fake_hal_reset();
    Library *library = fixture_library_open(&favorite_fixture);
    favorites_store_load_tracks(&favorite_store, library);
    player_init(&favorite_player, library);
    favorite_target = (Screen){0};
    favorite_toast = (Toast){0};
    TrackListConfig config = {
        .library = library,
        .player = &favorite_player,
        .now_playing = &favorite_target,
        .theme = &DEFAULT_THEME,
        .source = favorite_track_source(&favorite_source, &favorite_store, &favorite_toast),
        .title = STR_MENU_FAVORITES,
        .empty_message = STR_MENU_NO_SONG_FAVORITES,
    };
    Screen *screen = track_list_screen_init(&favorite_tracks, config);
    screen->enter(screen->context, START_MS);
    return screen;
}

static void two_song_favorites(void)
{
    favorites_store_toggle_track(&favorite_store, 0);
    favorites_store_toggle_track(&favorite_store, 1);
}

static void favorite_tracks_lists_nothing_when_empty(void)
{
    Screen *screen = fresh_favorite_tracks();
    TrackListScreen *favorites = screen->context;
    CHECK_EQUAL(0, favorites->count);
    ScreenAction action = screen->handle_key(
        favorites, (KeyEvent){.key = KEY_PLAY, .gesture = KEY_GESTURE_PRESS}, START_MS);
    CHECK_EQUAL(SCREEN_ACTION_NONE, action.type);
}

static void favorite_tracks_lists_two_favorites_in_title_order(void)
{
    Screen *screen = fresh_favorite_tracks();
    two_song_favorites();
    screen->enter(screen->context, START_MS);
    TrackListScreen *favorites = screen->context;
    CHECK_EQUAL(2, favorites->count);
    LibraryTrack track;
    CHECK(library_read_track(favorites->config.library, favorites->tracks[0], &track));
    char title[TRACK_LIST_LABEL_CAPACITY];
    CHECK(library_read_string(favorites->config.library, track.title, title, sizeof title));
    CHECK(strcmp(title, favorites->labels[0]) == 0);
    CHECK(library_read_track(favorites->config.library, favorites->tracks[1], &track));
    CHECK(library_read_string(favorites->config.library, track.title, title, sizeof title));
    CHECK(strcmp(title, favorites->labels[1]) == 0);
    char duration[TRACK_LIST_DETAIL_CAPACITY];
    format_duration(track.duration_ms, duration, sizeof duration);
    CHECK(strcmp(duration, favorites->details[1]) == 0);
}

static void favorite_tracks_az_works_for_one_track(void)
{
    Screen *screen = fresh_favorite_tracks();
    favorites_store_toggle_track(&favorite_store, 0);
    screen->enter(screen->context, START_MS);
    CHECK_EQUAL(1, favorite_tracks.count);
    CHECK(screen->long_menu_enabled);
    ScreenAction opened = screen->long_menu(screen->long_menu_context, START_MS + 10);
    CHECK_EQUAL(SCREEN_ACTION_PUSH, opened.type);
    CHECK(strcmp("A-Z", opened.target->title) == 0);
}

static void favorite_tracks_support_az_jump(void)
{
    Screen *screen = fresh_favorite_tracks();
    favorites_store_toggle_track(&favorite_store, 0);
    favorites_store_toggle_track(&favorite_store, 6);
    screen->enter(screen->context, START_MS);
    TrackListScreen *favorites = screen->context;
    CHECK(favorites->model.section != NULL);
    CHECK(screen->long_menu_enabled);
    ScreenAction opened = screen->long_menu(screen->long_menu_context, START_MS + 10);
    CHECK_EQUAL(SCREEN_ACTION_PUSH, opened.type);
    CHECK(strcmp("A-Z", opened.target->title) == 0);
    SectionJumpScreen *jump = opened.target->context;
    int before = jump->selected;
    ScreenAction moved = opened.target->handle_key(
        opened.target->context, (KeyEvent){.key = KEY_MENU, .gesture = KEY_GESTURE_LONG_PRESS},
        START_MS + 15);
    CHECK_EQUAL(SCREEN_ACTION_NONE, moved.type);
    CHECK_EQUAL((before + 7) % (LIST_SECTION_AFTER_LETTERS + 1), jump->selected);
    jump->selected = 21;
    opened.target->handle_key(opened.target->context,
                              (KeyEvent){.key = KEY_MENU, .gesture = KEY_GESTURE_LONG_PRESS},
                              START_MS + 18);
    CHECK_EQUAL(0, jump->selected);
    ScreenAction chosen = opened.target->handle_key(
        opened.target->context, (KeyEvent){.key = KEY_PLAY, .gesture = KEY_GESTURE_PRESS},
        START_MS + 20);
    CHECK_EQUAL(SCREEN_ACTION_POP, chosen.type);
}

static void starting_the_second_favorite_uses_the_favorites_queue(void)
{
    Screen *screen = fresh_favorite_tracks();
    two_song_favorites();
    screen->enter(screen->context, START_MS);
    TrackListScreen *favorites = screen->context;
    screen->handle_key(favorites,
                       (KeyEvent){.key = KEY_VOLUME_UP, .gesture = KEY_GESTURE_PRESS},
                       START_MS + 10);
    ScreenAction action = screen->handle_key(
        favorites, (KeyEvent){.key = KEY_PLAY, .gesture = KEY_GESTURE_PRESS}, START_MS + 20);
    CHECK_EQUAL(SCREEN_ACTION_PUSH, action.type);
    CHECK(action.target == &favorite_target);
    PlayerStatus status = player_status(&favorite_player);
    CHECK_EQUAL(favorites->tracks[1], status.track);
    CHECK_EQUAL(2, status.place.total);
}

static BrowsePool lite_actions_pool;

static void song_actions_add_and_remove_in_lite(void)
{
    fake_hal_reset();
    Library *library = fixture_library_open(&favorite_fixture);
    favorites_store_load_tracks(&favorite_store, library);
    player_init(&favorite_player, library);
    favorite_target = (Screen){0};
    favorite_toast = (Toast){0};
    BrowsePoolConfig config = {
        .library = library,
        .theme = &DEFAULT_THEME,
        .player = &favorite_player,
        .now_playing = &favorite_target,
        .favorites = &favorite_store,
        .toast = &favorite_toast,
    };
    browse_pool_init(&lite_actions_pool, config);
    ScreenAction opened = browse_pool_song_actions(&lite_actions_pool, 0, "Track", START_MS);
    SongActions *actions = &lite_actions_pool.song_actions;
    CHECK_EQUAL(SCREEN_ACTION_PUSH, opened.type);
    CHECK(opened.target == &actions->menu.screen);
    CHECK_EQUAL(3, actions->menu.config.item_count);
    CHECK(strcmp(i18n_text(STR_MENU_ADD_TO_FAVORITES), actions->items[2].label) == 0);
    ScreenAction result = actions->items[2].select(actions->items[2].context);
    CHECK_EQUAL(SCREEN_ACTION_POP, result.type);
    CHECK(favorites_store_contains_track(&favorite_store, 0));
    browse_pool_song_actions(&lite_actions_pool, 0, "Track", START_MS);
    CHECK(strcmp(i18n_text(STR_MENU_REMOVE_FROM_FAVORITES), actions->items[2].label) == 0);
    actions->items[2].select(actions->items[2].context);
    CHECK(!favorites_store_contains_track(&favorite_store, 0));
}

static void favorite_tracks_do_not_rescan_on_every_enter(void)
{
    Screen *screen = fresh_favorite_tracks();
    favorites_store_toggle_track(&favorite_store, 0);
    screen->enter(screen->context, START_MS);
    CHECK_EQUAL(1, favorite_tracks.count);
    favorite_fixture.memory.reads = 0;
    screen->enter(screen->context, START_MS + 10);
    CHECK_EQUAL(0, favorite_fixture.memory.reads);
}

static void favorite_tracks_refresh_when_the_store_changes(void)
{
    Screen *screen = fresh_favorite_tracks();
    favorites_store_toggle_track(&favorite_store, 0);
    screen->enter(screen->context, START_MS);
    CHECK_EQUAL(1, favorite_tracks.count);
    favorites_store_toggle_track(&favorite_store, 1);
    screen->update(screen->context, START_MS + 10);
    CHECK_EQUAL(2, favorite_tracks.count);
}

static void collecting_many_favorites_services_audio(void)
{
    static BigLibrary big;
    fake_hal_reset();
    Library *library = big_library_open(&big, 40, 4);
    favorites_store_load_tracks(&favorite_store, library);
    for (uint16_t track = 0; track < 20; track++) {
        favorites_toggle(&favorite_store.favorites, library_track_key(library, track));
    }
    favorite_store.revision++;
    player_init(&favorite_player, library);
    favorite_target = (Screen){0};
    favorite_toast = (Toast){0};
    TrackListConfig config = {
        .library = library,
        .player = &favorite_player,
        .now_playing = &favorite_target,
        .theme = &DEFAULT_THEME,
        .source = favorite_track_source(&favorite_source, &favorite_store, &favorite_toast),
        .title = STR_MENU_FAVORITES,
        .empty_message = STR_MENU_NO_SONG_FAVORITES,
    };
    Screen *screen = track_list_screen_init(&favorite_tracks, config);
    int before = fake_hal_audio_service_count();
    screen->enter(screen->context, START_MS);
    CHECK(fake_hal_audio_service_count() > before);
    big_library_close(&big);
}

static void long_play_removes_a_song_favorite(void)
{
    Screen *screen = fresh_favorite_tracks();
    two_song_favorites();
    screen->enter(screen->context, START_MS);
    TrackListScreen *favorites = screen->context;
    screen->handle_key(favorites,
                       (KeyEvent){.key = KEY_PLAY, .gesture = KEY_GESTURE_LONG_PRESS},
                       START_MS + 10);
    CHECK_EQUAL(1, favorites->count);
    CHECK(favorite_toast.visible);
    CHECK_EQUAL(1, favorite_store.favorites.count);
}

static void settings_starts_on_home(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(300);
    CHECK(title_is("Settings"));
    CHECK(area_has_color(18, 38, 10, 10, COLOR_RGB(255, 255, 255)));
    CHECK(area_has_color(18, 70, 10, 10, COLOR_RGB(99, 102, 241)));
    CHECK(area_has_color(18, 102, 10, 10, COLOR_RGB(47, 168, 255)));
    CHECK(area_has_color(18, 134, 10, 10, COLOR_RGB(53, 199, 232)));
    CHECK(area_has_color(240, 29, 70, 28, COLOR_RGB(255, 255, 255)));
    CHECK(area_has_color(240, 61, 70, 28, DEFAULT_THEME.colors->text_secondary));
    move_down(4);
    wait_ms(200);
    CHECK(area_has_color(18, 38, 10, 10, COLOR_RGB(99, 102, 241)));
    CHECK(fake_hal_pixel(160, 139) == DEFAULT_THEME.colors->selection_background);
    CHECK(fake_hal_pixel(160, 152) == DEFAULT_THEME.colors->selection_background);
}

static void landscape_settings_has_no_title(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(300);
    CHECK(title_is("Settings"));
    CHECK(fake_hal_pixel(20, 16) == COLOR_RGB(250, 250, 249));
}

static void settings_category_back_returns_home(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(200);
    int home_depth = app_screen_depth();
    move_down(2);
    tap(KEY_PLAY);
    wait_ms(200);
    CHECK_EQUAL(home_depth + 1, app_screen_depth());
    CHECK(fake_hal_pixel(254, 8) == DEFAULT_THEME.colors->background);
    tap(KEY_BACK);
    CHECK_EQUAL(home_depth, app_screen_depth());
    tap(KEY_BACK);
    CHECK_EQUAL(home_depth - 1, app_screen_depth());
    CHECK(title_is("All Songs"));
}

static void long_back_from_a_settings_choice_returns_root(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(200);
    move_down(3);
    tap(KEY_PLAY);
    wait_ms(200);
    move_down(2);
    tap(KEY_PLAY);
    wait_ms(200);
    CHECK(title_is("Brightness"));
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("All Songs"));
}

static void long_back_from_equalizer_custom_returns_root(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(200);
    move_down(2);
    tap(KEY_PLAY);
    wait_ms(200);
    move_down(5);
    tap(KEY_PLAY);
    wait_ms(200);
    CHECK(title_is("Equalizer"));
    move_down(7);
    tap(KEY_PLAY);
    wait_ms(200);
    int depth = app_screen_depth();
    CHECK(depth > 1);
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("All Songs"));
}

static void long_back_from_folders_returns_root(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(200);
    move_down(4);
    tap(KEY_PLAY);
    wait_ms(200);
    move_down(6);
    tap(KEY_PLAY);
    wait_ms(200);
    CHECK(!title_is("Settings"));
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("All Songs"));
}

static void long_back_from_date_time_returns_root(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(200);
    move_down(4);
    tap(KEY_PLAY);
    wait_ms(200);
    move_down(1);
    tap(KEY_PLAY);
    wait_ms(200);
    CHECK(!title_is("Settings"));
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("All Songs"));
}

static void long_back_from_a_settings_subpage_returns_root(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(200);
    move_down(2);
    tap(KEY_PLAY);
    wait_ms(200);
    move_down(5);
    tap(KEY_PLAY);
    wait_ms(200);
    CHECK(title_is("Equalizer"));
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("All Songs"));
}

static void settings_long_menu_does_nothing(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(300);
    hold(KEY_MENU);
    wait_ms(200);
    CHECK(title_is("Settings"));
    CHECK(fake_hal_pixel(12, 41) == COLOR_RGB(0, 111, 238));
}

static void settings_short_press_cycles_value(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(300);
    unsigned before = area_hash(8, 29, 304, 28);
    int depth = app_screen_depth();
    tap(KEY_PLAY);
    wait_ms(100);
    CHECK(title_is("Settings"));
    CHECK_EQUAL(depth, app_screen_depth());
    CHECK(area_hash(8, 29, 304, 28) != before);
}

static void settings_subpage_opens(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(300);
    move_down(2);
    tap(KEY_PLAY);
    wait_ms(200);
    CHECK(title_is("Settings"));
    move_down(5);
    tap(KEY_PLAY);
    wait_ms(100);
    CHECK(title_is("Equalizer"));
    tap(KEY_BACK);
    CHECK(title_is("Settings"));
}

static void open_all_songs_and_play(void)
{
    CHECK(title_is("All Songs"));
    tap(KEY_PLAY);
    CHECK(title_is("Now Playing"));
}

static void now_playing_uses_a_status_mode_block(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    wait_ms(300);
    CHECK(!area_has_color(294, 24, 18, 18, COLOR_RGB(0, 111, 238)));
    CHECK(fake_hal_pixel(302, 10) == DEFAULT_THEME.colors->text_tertiary);
    CHECK(fake_hal_pixel(304, 10) == DEFAULT_THEME.colors->text_tertiary);
    CHECK(!area_has_color(8, 64, 304, 18, COLOR_RGB(0, 111, 238)));
    CHECK(fake_hal_pixel(40, 145) == DEFAULT_THEME.colors->divider);
    tap(KEY_PLAY);
    wait_ms(300);
    CHECK(fake_hal_pixel(302, 10) == DEFAULT_THEME.colors->text_tertiary);
}

static void all_songs_long_play_opens_song_actions(void)
{
    start_app_with_library();
    CHECK(title_is("All Songs"));
    int depth = app_screen_depth();
    hold(KEY_PLAY);
    CHECK_EQUAL(depth + 1, app_screen_depth());
    move_down(2);
    tap(KEY_PLAY);
    CHECK_EQUAL(depth, app_screen_depth());
    CHECK(title_is("All Songs"));
}

static void short_menu_in_settings_returns_to_now_playing(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    int depth = app_screen_depth();
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("Settings"));
    CHECK_EQUAL(depth + 1, app_screen_depth());
    tap(KEY_MENU);
    CHECK(title_is("Now Playing"));
    CHECK_EQUAL(depth, app_screen_depth());
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("Settings"));
    tap(KEY_MENU);
    CHECK(title_is("Now Playing"));
    CHECK_EQUAL(depth, app_screen_depth());
}

static void long_menu_on_now_playing_opens_song_actions(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    int depth = app_screen_depth();
    hold(KEY_MENU);
    CHECK_EQUAL(depth + 1, app_screen_depth());
    tap(KEY_BACK);
    CHECK(title_is("Now Playing"));
    CHECK_EQUAL(depth, app_screen_depth());
}

static void long_play_on_now_playing_does_nothing(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    hold(KEY_PLAY);
    CHECK(title_is("Now Playing"));
    CHECK_EQUAL(2, app_screen_depth());
}

static void settings_menu_goes_to_now_playing_and_drops_settings(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("Settings"));
    tap(KEY_MENU);
    wait_ms(300);
    CHECK(title_is("Now Playing"));
    tap(KEY_BACK);
    CHECK(title_is("All Songs"));
}

static void lite_ignores_the_menu_key_setting(void)
{
    uint8_t options[SETTINGS_OPTION_COUNT] = {0};
    options[OPTION_MENU_KEY] = MENU_KEY_NEXT;
    CHECK(!now_playing_controls_of(options).menu_skips);
}

static void long_menu_opens_and_closes_section_jump(void)
{
    start_app_with_library();
    CHECK(title_is("All Songs"));
    hold(KEY_MENU);
    CHECK(title_is("A-Z"));
    tap(KEY_BACK);
    CHECK(title_is("All Songs"));
}

static void short_menu_in_az_opens_now_playing(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    tap(KEY_BACK);
    CHECK(title_is("All Songs"));
    hold(KEY_MENU);
    CHECK(title_is("A-Z"));
    tap(KEY_MENU);
    CHECK(title_is("Now Playing"));
}

static void long_menu_in_az_moves_one_row(void)
{
    start_app_with_library();
    hold(KEY_MENU);
    CHECK(title_is("A-Z"));
    hold(KEY_MENU);
    wait_ms(100);
    CHECK(title_is("A-Z"));
    tap(KEY_PLAY);
    CHECK(title_is("All Songs"));
}

static void long_back_in_az_opens_settings(void)
{
    start_app_with_library();
    hold(KEY_MENU);
    CHECK(title_is("A-Z"));
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("Settings"));
}

static void long_menu_in_up_next_does_nothing(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    tap(KEY_MENU);
    CHECK(title_is("Up Next"));
    int depth = app_screen_depth();
    hold(KEY_MENU);
    wait_ms(100);
    CHECK(title_is("Up Next"));
    CHECK_EQUAL(depth, app_screen_depth());
}

static void section_jump_play_returns_to_the_list(void)
{
    start_app_with_library();
    hold(KEY_MENU);
    CHECK(title_is("A-Z"));
    move_down(4);
    tap(KEY_PLAY);
    CHECK(title_is("All Songs"));
}

static void short_menu_in_a_lettered_list_opens_now_playing(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    tap(KEY_BACK);
    CHECK(title_is("All Songs"));
    tap(KEY_MENU);
    CHECK(title_is("Now Playing"));
}

static void long_back_in_settings_returns_to_root(void)
{
    start_app_with_library();
    open_all_songs_and_play();
    hold(KEY_BACK);
    wait_ms(200);
    CHECK(title_is("Settings"));
    hold(KEY_BACK);
    CHECK(title_is("All Songs"));
}

static void open_settings_display(void)
{
    start_app_with_library();
    hold(KEY_BACK);
    wait_ms(200);
    move_down(3);
    tap(KEY_PLAY);
    wait_ms(200);
}

static void switch_orientation_to_portrait(void)
{
    open_settings_display();
    move_down(1);
    tap(KEY_PLAY);
    wait_ms(300);
    tap(KEY_BACK);
    wait_ms(200);
    tap(KEY_BACK);
    wait_ms(300);
}

static void switch_theme_to_dark(void)
{
    open_settings_display();
    tap(KEY_PLAY);
    wait_ms(300);
    tap(KEY_BACK);
    wait_ms(200);
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
    CHECK(lite_card_shadow_color(&dark) == COLOR_RGB(0, 0, 0));
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
    CHECK(title_is("All Songs"));
    CHECK(fake_hal_pixel(8, 30) == DEFAULT_THEME.colors->text);
    tap(KEY_PLAY);
    wait_ms(300);
    CHECK(title_is("Now Playing"));
    CHECK(fake_hal_pixel(40, 289) == DEFAULT_THEME.colors->divider);
}

static void settings_can_switch_to_portrait_flipped(void)
{
    open_settings_display();
    move_down(1);
    tap(KEY_PLAY);
    wait_ms(300);
    CHECK_EQUAL(HAL_DISPLAY_PORTRAIT, fake_hal_display_rotation());
    tap(KEY_PLAY);
    wait_ms(300);
    CHECK_EQUAL(HAL_DISPLAY_PORTRAIT_FLIPPED, fake_hal_display_rotation());
}

static void settings_can_switch_to_dark(void)
{
    switch_theme_to_dark();
    CHECK(title_is("All Songs"));
    CHECK(fake_hal_pixel(40, 16) == COLOR_RGB(11, 11, 13));
    CHECK(fake_hal_pixel(120, 40) == COLOR_RGB(23, 23, 28));
    CHECK(fake_hal_pixel(8, 30) == COLOR_RGB(82, 82, 91));
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
    wait_ms(200);
    tap(KEY_BACK);
    wait_ms(300);
}

static void dark_portrait_root_and_settings(void)
{
    switch_dark_portrait();
    CHECK(title_is("All Songs"));
    CHECK(fake_hal_pixel(40, 16) == COLOR_RGB(11, 11, 13));
    CHECK(fake_hal_pixel(85, 110) == COLOR_RGB(23, 23, 28));
    CHECK(fake_hal_pixel(8, 30) == COLOR_RGB(82, 82, 91));

    tap(KEY_PLAY);
    wait_ms(300);
    CHECK(title_is("Now Playing"));
}

static void portrait_now_playing_icons(void)
{
    switch_orientation_to_portrait();
    open_all_songs_and_play();
    wait_ms(300);
    CHECK(fake_hal_pixel(152, 10) == DEFAULT_THEME.colors->text_tertiary);
    CHECK(fake_hal_pixel(154, 10) == DEFAULT_THEME.colors->text_tertiary);
    CHECK(!area_has_color(144, 24, 18, 18, COLOR_RGB(0, 111, 238)));
    CHECK(fake_hal_pixel(40, 289) == DEFAULT_THEME.colors->divider);
}

static void boot_has_portrait_still(void)
{
    CHECK(load_animation_asset());
    lite_animation_reset();
    static Color pixels[HAL_DISPLAY_WIDTH * HAL_DISPLAY_LONG_SIDE];
    Canvas canvas = canvas_make(pixels, (Rect){0, 0, 170, 320});
    canvas_clear(&canvas, COLOR_RGB(250, 250, 249));
    CHECK(lite_boot_draw(&canvas, (Rect){0, 0, 170, 320}, COLOR_RGB(10, 10, 10), true));
    CHECK(canvas_pixel_at(&canvas, 1, 1) == COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 11, 104) == COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 12, 104) != COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 157, 215) != COLOR_RGB(250, 250, 249));
    CHECK(canvas_pixel_at(&canvas, 162, 220) == COLOR_RGB(10, 10, 10));
    CHECK(canvas_pixel_at(&canvas, 163, 221) == COLOR_RGB(250, 250, 249));
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
        TEST_CASE(boot_starts_on_all_songs),
        TEST_CASE(a_saved_start_screen_still_boots_all_songs),
        TEST_CASE(now_playing_uses_a_status_mode_block),
        TEST_CASE(all_songs_long_play_opens_song_actions),

        TEST_CASE(short_menu_in_settings_returns_to_now_playing),
        TEST_CASE(long_menu_on_now_playing_opens_song_actions),
        TEST_CASE(long_play_on_now_playing_does_nothing),
        TEST_CASE(settings_menu_goes_to_now_playing_and_drops_settings),
        TEST_CASE(lite_ignores_the_menu_key_setting),
        TEST_CASE(long_menu_opens_and_closes_section_jump),
        TEST_CASE(short_menu_in_az_opens_now_playing),
        TEST_CASE(long_menu_in_az_moves_one_row),
        TEST_CASE(long_back_in_az_opens_settings),
        TEST_CASE(long_menu_in_up_next_does_nothing),
        TEST_CASE(section_jump_play_returns_to_the_list),
        TEST_CASE(short_menu_in_a_lettered_list_opens_now_playing),
        TEST_CASE(long_back_in_settings_returns_to_root),
        TEST_CASE(gear_glyph_draws_only_white_outline),
        TEST_CASE(every_lite_icon_draws),
        TEST_CASE(badge_draws_flat_circle_and_half),
        TEST_CASE(category_dots_have_no_cardinal_spikes),
        TEST_CASE(dark_palette_and_borders),
        TEST_CASE(status_blocks_draw_flat_squares),
        TEST_CASE(status_mode_block_centers_the_one),
        TEST_CASE(status_mode_block_cycles_gray_one_and_purple),
        TEST_CASE(settings_can_switch_to_portrait),
        TEST_CASE(settings_can_switch_to_portrait_flipped),
        TEST_CASE(settings_can_switch_to_dark),
        TEST_CASE(dark_portrait_root_and_settings),
        TEST_CASE(portrait_now_playing_icons),
        TEST_CASE(boot_has_portrait_still),
        TEST_CASE(charge_card_is_portrait),
        TEST_CASE(category_icons_are_40px),
        TEST_CASE(compact_icons_are_native_18px),
        TEST_CASE(favorite_tracks_lists_nothing_when_empty),
        TEST_CASE(favorite_tracks_lists_two_favorites_in_title_order),
        TEST_CASE(favorite_tracks_az_works_for_one_track),
        TEST_CASE(favorite_tracks_support_az_jump),
        TEST_CASE(starting_the_second_favorite_uses_the_favorites_queue),
        TEST_CASE(long_play_removes_a_song_favorite),
        TEST_CASE(song_actions_add_and_remove_in_lite),
        TEST_CASE(favorite_tracks_do_not_rescan_on_every_enter),
        TEST_CASE(favorite_tracks_refresh_when_the_store_changes),
        TEST_CASE(collecting_many_favorites_services_audio),
        TEST_CASE(settings_starts_on_home),
        TEST_CASE(landscape_settings_has_no_title),
        TEST_CASE(settings_category_back_returns_home),
        TEST_CASE(long_back_from_a_settings_subpage_returns_root),
        TEST_CASE(long_back_from_a_settings_choice_returns_root),
        TEST_CASE(long_back_from_equalizer_custom_returns_root),
        TEST_CASE(long_back_from_folders_returns_root),
        TEST_CASE(long_back_from_date_time_returns_root),
        TEST_CASE(settings_long_menu_does_nothing),
        TEST_CASE(settings_short_press_cycles_value),
        TEST_CASE(settings_subpage_opens),
        TEST_CASE(lite_animation_arena_is_available),
        TEST_CASE(lite_animation_streams_all_frames),
        TEST_CASE(no_title_list_frame_starts_below_the_meter),
        TEST_CASE(cjk_section_initials_follow_pinyin),
        TEST_CASE(boot_screen_is_a_shadowed_image),
        TEST_CASE(boot_screen_draws_the_still),
        TEST_CASE(boot_screen_is_static),
        TEST_CASE(boot_draws_across_strips),
        TEST_CASE(back_at_the_root_toggles_all_songs_and_favorites),
        TEST_CASE(back_from_now_playing_returns_to_the_current_root),
        TEST_CASE(settings_from_favorites_returns_to_favorites),
        TEST_CASE(long_back_on_now_playing_opens_settings),
        TEST_CASE(back_from_now_playing_returns_to_favorites_root),
        TEST_CASE(long_back_on_favorites_opens_settings),
        TEST_CASE(root_status_blocks_track_bluetooth_and_root),
        TEST_CASE(now_playing_uses_bt_chip_with_controls),
        TEST_CASE(now_playing_has_no_time_or_thumb),
        TEST_CASE(hold_the_volume_key_seeks_without_a_hold_bar),
        TEST_CASE(date_time_uses_a_settings_card),
        TEST_CASE(goodbye_screen_has_no_icon),
        TEST_CASE(the_sleep_timer_waits_for_the_current_track),
        TEST_CASE(the_sleep_quick_card_counts_down),
        TEST_CASE(charging_screen_has_green_shadow_and_no_text),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
