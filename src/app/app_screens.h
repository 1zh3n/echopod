#pragma once

#include <stdbool.h>

#include "browse_screen.h"
#include "core/power_policy.h"
#include "cover_cache.h"
#ifndef ECHO_LITE
#include "diagnostics_values.h"
#endif
#include "favorites_store.h"
#include "hal/audio.h"
#include "home_art.h"
#include "library_store.h"
#include "player.h"
#include "screens/bluetooth_screen.h"
#include "screens/cover_flow_screen.h"
#include "screens/date_time_screen.h"
#include "screens/equalizer_screen.h"
#include "screens/now_playing_screen.h"
#ifdef ECHO_LITE
#include "screens/playback_panel_screen.h"
#endif
#ifndef ECHO_LITE
#include "screens/test_pattern_screen.h"
#endif
#include "screens/up_next_screen.h"
#include "settings/settings.h"
#include "settings_screens.h"
#include "settings_store.h"
#include "ui/menu_screen.h"
#include "ui/message_screen.h"
#include "ui/toast.h"

enum {
#ifdef ECHO_LITE
    HOME_ITEM_COUNT = 3,
#else
    HOME_ITEM_COUNT = 6,
#endif
    SHELF_COUNT = 2,
    MUSIC_ITEM_COUNT = 9,
    DIAGNOSTICS_ITEM_COUNT = 23,
    START_PATH_CAPACITY = 2
};

typedef struct AppOptions {
    bool show_frame_time;
    bool ignore_power_taps;
} AppOptions;

#ifndef ECHO_LITE
typedef struct ClockOption {
    bool failed;
} ClockOption;

typedef struct Core1AccelOption {
    bool run;
    HalCore1Accel result;
} Core1AccelOption;
#endif

typedef struct BrowseEntry {
    BrowsePool *pool;
    const LibraryStore *store;
    Screen *unavailable;
    BrowseQuery query;
    bool favorites;
    CoverFlowScreen *flow;
    SpokenKind shelf;
    bool present;
} BrowseEntry;

typedef struct AppScreens {
    MenuScreen home;
    MenuScreen music;
    SettingsScreens settings;
    MenuScreen diagnostics;
    MessageScreen no_library;
    BrowsePool browse;
#ifdef ECHO_LITE
    PlaybackPanelScreen playback_panel;
#endif
    NowPlayingScreen now_playing;
    UpNextScreen up_next;
    CoverFlowScreen cover_flow;
    BrowseEntry browse_entries[MUSIC_ITEM_COUNT];
#ifndef ECHO_LITE
    BrowseEntry shelf_entries[SHELF_COUNT];
    TestPatternScreen test_pattern;
    ClockOption clock;
    Core1AccelOption core1_accel;
#endif
    BluetoothScreen bluetooth;
    EqualizerScreen equalizer;
    DateTimeScreen date_time;
    FoldersScreen folders;
    MenuItem home_items[HOME_ITEM_COUNT];
#ifdef ECHO_LITE
    const Player *player;
#endif
    HomeArt home_art;
    MenuScene home_scene;
    MenuItem music_items[MUSIC_ITEM_COUNT];
#ifndef ECHO_LITE
    MenuItem diagnostics_items[DIAGNOSTICS_ITEM_COUNT];
#endif
} AppScreens;

typedef struct AppScreensContext {
    AppOptions *options;
    const Theme *theme;
    const Theme *stage_theme;
    const LibraryStore *store;
    Player *player;
    SettingsStore *settings;
    FavoritesStore *favorites;
    PowerPolicy *power;
    Toast *toast;
    CoverCache *covers;
    bool *rescan_requested;
    SpokenStore *spoken;
    BluetoothLink *bluetooth;
    EqualizerStore *equalizer;
    ClockStore *clock;
} AppScreensContext;

Screen *app_screens_init(AppScreens *screens, AppScreensContext context);
MenuItem app_screens_open_item(const char *label, Screen *target);
Screen *app_screens_build_menu(MenuScreen *menu, const char *title, const MenuItem *items,
                               int count, const Theme *theme);
#ifndef ECHO_LITE
void diagnostics_menu_build(AppScreens *screens, AppOptions *options, const Theme *theme);
#endif
int app_screens_start_path(AppScreens *screens, StartScreen start, Screen **path);
#ifdef ECHO_LITE
bool app_screens_home_key(AppScreens *screens, KeyEvent event, uint32_t now_ms);
#endif
