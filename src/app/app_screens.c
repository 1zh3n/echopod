#include "app_screens.h"
#include "core/resident.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/i18n.h"
#include "diagnostics_values.h"
#include "hal/power.h"
#include "hal/storage.h"
#include "hal/system.h"
#include "theme_store.h"
#include "ui/box.h"
#include "ui/icons.h"
#include "ui/label.h"

typedef enum MusicSource {
    MUSIC_COVER_FLOW,
    MUSIC_LIBRARY,
    MUSIC_FAVORITES
} MusicSource;

typedef struct MusicEntry {
    StringId label;
    MusicSource source;
    BrowseKind kind;
    bool keep;
} MusicEntry;

enum {
    START_HOME_ONLY = -1
};

static const int8_t START_ENTRIES[START_SCREEN_COUNT] = {
    [START_MAIN_MENU] = START_HOME_ONLY,
    [START_NOW_PLAYING] = START_HOME_ONLY,
    [START_ALBUMS] = 3,
    [START_ARTISTS] = 1,
    [START_FAVORITES] = 8,
    [START_PLAYLISTS] = 7,
    [START_COVER_FLOW] = 0,
    [START_ALBUM_ARTISTS] = 2,
    [START_GENRES] = 4,
    [START_FOLDERS] = 5,
    [START_ALL_SONGS] = 6,
};

static const MusicEntry MUSIC_ENTRIES[MUSIC_ITEM_COUNT] = {
    {STR_MENU_COVER_FLOW, MUSIC_COVER_FLOW, BROWSE_ALBUMS, false},
    {STR_MENU_ARTISTS, MUSIC_LIBRARY, BROWSE_ARTISTS, false},
    {STR_MENU_ALBUM_ARTISTS, MUSIC_LIBRARY, BROWSE_ALBUM_ARTISTS, false},
    {STR_MENU_ALBUMS, MUSIC_LIBRARY, BROWSE_ALBUMS, false},
    {STR_MENU_GENRES, MUSIC_LIBRARY, BROWSE_GENRES, false},
    {STR_MENU_FOLDERS, MUSIC_LIBRARY, BROWSE_FOLDER, true},
    {STR_MENU_ALL_SONGS, MUSIC_LIBRARY, BROWSE_ALL_SONGS, true},
    {STR_MENU_PLAYLISTS, MUSIC_LIBRARY, BROWSE_PLAYLISTS, true},
    {STR_MENU_FAVORITES, MUSIC_FAVORITES, BROWSE_ALBUMS, false},
};

#ifdef ECHO_LITE
static bool music_entry_shown(const void *context)
{
    uintptr_t index = (uintptr_t)context;
    return index < MUSIC_ITEM_COUNT && MUSIC_ENTRIES[index].keep;
}
#endif

static int music_position(int entry)
{
#ifdef ECHO_LITE
    int position = 0;
    for (int index = 0; index < entry; index++) {
        position += MUSIC_ENTRIES[index].keep ? 1 : 0;
    }
    return position;
#else
    return entry;
#endif
}

#ifndef ECHO_LITE
static bool player_has_track(const void *context)
{
    return player_status(context).has_track;
}
#endif

MenuItem app_screens_open_item(const char *label, Screen *target)
{
    return (MenuItem){.label = label,
                      .select = menu_open_screen,
                      .context = target,
                      .value = NULL,
                      .opens = true};
}

Screen *app_screens_build_menu(MenuScreen *menu, const char *title, const MenuItem *items,
                               int count, const Theme *theme)
{
    return menu_screen_init(
        menu, (MenuConfig){.title = title, .items = items, .item_count = count, .theme = theme});
}

static Screen *browse_screen_for(BrowseEntry *entry)
{
    if (!entry->store->available) {
        return entry->unavailable;
    }
#ifndef ECHO_LITE
    if (entry->flow != NULL && entry->query.kind != BROWSE_FOLDER) {
        return cover_flow_screen_open_all(entry->flow);
    }
    const uint8_t *options = entry->pool->config.options;
    if (entry->flow != NULL && options != NULL && options[OPTION_FOLDER_VIEW] != 0) {
        Screen *folders = cover_flow_screen_open_group(entry->flow, LIBRARY_FOLDERS, 0, 0);
        if (folders != NULL) {
            return folders;
        }
    }
#endif
#ifndef ECHO_LITE
    if (entry->favorites) {
        return browse_pool_open_favorites(entry->pool, 0);
    }
#endif
#ifndef ECHO_LITE
    if (entry->shelf != SPOKEN_NONE) {
        return browse_pool_open_shelf(entry->pool, 0, entry->shelf);
    }
#endif
    return browse_pool_open(entry->pool, 0, entry->query);
}

static ScreenAction open_browse(void *context)
{
    return screen_action_push(browse_screen_for(context));
}

static MenuItem music_item(AppScreens *screens, const LibraryStore *store, int index)
{
    const MusicEntry *entry = &MUSIC_ENTRIES[index];
    screens->browse_entries[index] = (BrowseEntry){
        .pool = &screens->browse,
        .store = store,
        .unavailable = &screens->no_library.screen,
        .query = browse_root_query(entry->kind),
        .favorites = entry->source == MUSIC_FAVORITES,
#ifdef ECHO_LITE
        .flow = NULL,
#else
        .flow = entry->source == MUSIC_COVER_FLOW || entry->kind == BROWSE_FOLDER
                    ? &screens->cover_flow
                    : NULL,
#endif
        .shelf = SPOKEN_NONE,
        .present = true,
    };
    return (MenuItem){.label = i18n_text(entry->label),
                      .select = open_browse,
                      .context = &screens->browse_entries[index],
                      .value = NULL,
                      .opens = true,
#ifdef ECHO_LITE
                      .shown = music_entry_shown,
                      .shown_context = (void *)(uintptr_t)index,
#endif
    };
}

#ifndef ECHO_LITE
static Screen *open_group_flow(void *context, LibrarySection group, uint16_t subject, int level)
{
    return cover_flow_screen_open_group(context, group, subject, level);
}
#endif

static void build_music(AppScreens *screens, AppScreensContext context)
{
    BrowsePoolConfig browse = {
        .library = &context.store->library,
        .theme = context.theme,
        .player = context.player,
        .now_playing = &screens->now_playing.screen,
        .favorites = context.favorites,
        .toast = context.toast,
#ifdef ECHO_LITE
        .spoken = NULL,
#else
        .spoken = context.spoken,
#endif
        .options = context.settings->settings.options,
#ifdef ECHO_LITE
        .open_flow = NULL,
        .flow_context = NULL,
#else
        .open_flow = open_group_flow,
        .flow_context = &screens->cover_flow,
#endif
    };
    browse_pool_init(&screens->browse, browse);
#ifndef ECHO_LITE
    CoverFlowConfig cover_flow = {
        .library = &context.store->library,
        .theme = context.stage_theme != NULL ? context.stage_theme : context.theme,
        .covers = context.covers,
        .favorites = context.favorites,
        .toast = context.toast,
        .browse = &screens->browse,
        .player = context.player,
        .options = context.settings->settings.options,
    };
    cover_flow_screen_init(&screens->cover_flow, cover_flow);
#endif
    for (int index = 0; index < MUSIC_ITEM_COUNT; index++) {
        screens->music_items[index] = music_item(screens, context.store, index);
    }
    app_screens_build_menu(&screens->music, i18n_text(STR_RESCAN_MUSIC_TITLE), screens->music_items,
                           MUSIC_ITEM_COUNT, context.theme);
}

static void build_settings(AppScreens *screens, AppScreensContext context)
{
    SettingsContext settings = {
        .store = context.settings,
        .player = context.player,
        .power = context.power,
        .toast = context.toast,
        .rescan_requested = context.rescan_requested,
        .bluetooth = &screens->bluetooth,
        .equalizer = &screens->equalizer,
        .date_time = &screens->date_time,
        .folders = &screens->folders,
        .sleep_option = 0,
    };
    DateTimeConfig date_time = {.clock = context.clock,
                                .store = context.settings,
                                .toast = context.toast,
                                .theme = context.theme};
    date_time_screen_init(&screens->date_time, date_time);
    FoldersConfig folders = {.toast = context.toast,
                             .rescan_requested = context.rescan_requested,
                             .theme = context.theme};
    folders_screen_init(&screens->folders, folders);
    settings_screens_init(&screens->settings, settings, context.theme);
}

#ifndef ECHO_LITE
static bool shelf_present(const void *context)
{
    const BrowseEntry *entry = context;
    return entry->present;
}
#endif

#ifndef ECHO_LITE
static MenuItem shelf_item(AppScreens *screens, AppScreensContext context, SpokenKind shelf)
{
    uint16_t folder = 0;
    BrowseEntry *entry = &screens->shelf_entries[shelf == SPOKEN_PODCAST ? 1 : 0];
    *entry = (BrowseEntry){
        .pool = &screens->browse,
        .store = context.store,
        .unavailable = &screens->no_library.screen,
        .shelf = shelf,
        .present = context.store->available &&
                   spoken_shelf_folder(&context.store->library, shelf, &folder),
    };
    return (MenuItem){
        .label = i18n_text(shelf == SPOKEN_PODCAST ? STR_MENU_PODCASTS : STR_MENU_AUDIOBOOKS),
        .select = open_browse,
        .context = entry,
        .opens = true,
        .shown = shelf_present,
        .shown_context = entry};
}
#endif

#ifndef ECHO_LITE
_Static_assert(THEME_ICON_COUNT - THEME_ICON_HOME_MUSIC == HOME_ITEM_COUNT,
               "Home has an icon for each of its items");
#endif

#ifdef ECHO_LITE
static int32_t home_position;
static int32_t home_target;

static bool home_scene_update(void *context, uint32_t now_ms)
{
    AppScreens *screens = context;
    (void)now_ms;
    home_target = screens->home.list.selected * 256;
    int32_t delta = home_target - home_position;
    if (delta != 0) {
        int32_t step = delta / 3;
        if (step == 0) {
            step = delta > 0 ? 1 : -1;
        }
        home_position += step;
    }
    return true;
}

static const ThemeIconId HOME_ICONS[HOME_ITEM_COUNT] = {
    THEME_ICON_HOME_MUSIC, THEME_ICON_HOME_AUDIOBOOKS, THEME_ICON_HOME_PODCASTS,
    THEME_ICON_HOME_NOW_PLAYING, THEME_ICON_HOME_SETTINGS};

static bool home_scene_draw(void *context, Canvas *canvas, MenuScreen *menu)
{
    (void)context;
    const Theme *theme = menu->config.theme;
    int width = theme->metrics.screen_width;
    int height = theme->metrics.screen_height;
    canvas_fill_rect(canvas, (Rect){0, 0, width, height}, theme->colors->background);
    int count = menu->config.item_count;
    int spacing = width / 5;
    Rect box = {width / 2 - 34, 26, 68, 92};
    box_draw(canvas, theme->colors, &theme->boxes->selection, box);
    int active = -1;
    for (int index = 0; index < count; index++) {
        int32_t relative = (int32_t)index * 256 - home_position;
        int x = width / 2 + (int)(relative * spacing / 256);
        bool selected = relative > -128 && relative < 128;
        int size = selected ? 32 : 24;
        int y = selected ? 38 : 46;
        Icon icon = {HOME_ICONS[index], (Rect){x - size / 2, y, size, size},
                     selected ? theme->colors->selection_text : theme->colors->text_secondary};
        icon_draw(canvas, theme, icon);
        if (selected) {
            active = index;
        }
    }
    if (active >= 0) {
        TextStyle style = {theme->fonts.body, theme->colors->selection_text};
        const char *text = menu->config.items[active].label;
        Rect label_area = {0, 116, width, 28};
        label_draw(canvas, style,
                   (Label){label_centered_area(style.font, label_area, text), text});
    }
    return true;
}

static void build_home(AppScreens *screens, AppScreensContext context)
{
    const Theme *theme = context.theme;
    screens->home_items[0] = music_item(screens, context.store, 5);
    screens->home_items[1] = music_item(screens, context.store, 6);
    screens->home_items[2] = music_item(screens, context.store, 7);
    screens->home_items[3] =
        app_screens_open_item(i18n_text(STR_MENU_NOW_PLAYING), &screens->now_playing.screen);
    screens->home_items[4] =
        app_screens_open_item(i18n_text(STR_MENU_SETTINGS), &screens->settings.menu.screen);
    home_position = 0;
    home_target = 0;
    screens->home_scene = (MenuScene){home_scene_update, home_scene_draw, screens, true};
    MenuConfig home = {.title = "EchoPod",
                       .items = screens->home_items,
                       .item_count = HOME_ITEM_COUNT,
                       .theme = theme,
                       .scene = &screens->home_scene};
    menu_screen_init(&screens->home, home);
    screens->home.list.visible_rows = HOME_ITEM_COUNT;
}
#else
static bool home_scene_update(void *context, uint32_t now_ms)
{
    AppScreens *screens = context;
    if (!theme_store_has_scene(THEME_SCENE_HOME)) {
        return false;
    }
    now_playing_follow(&screens->now_playing, now_ms);
    home_art_update(&screens->home_art, now_ms);
    return true;
}

static bool home_scene_draw(void *context, Canvas *canvas, MenuScreen *menu)
{
    const AppScreens *screens = context;
    const HomeArt *art = &screens->home_art;
    SceneMenu items = {&menu->model, menu->list.selected, menu_screen_scene_list,
                       menu_screen_scene_slot, menu};
    NowPlayingHome stage = {.title = menu->config.title, .menu = &items};
    if (!art->has_album ||
        cover_cache_full(art->covers, art->album, &stage.cover) != COVER_LOADED) {
        stage.cover = (Image){0};
    }
    return now_playing_draw_home(&screens->now_playing, &stage, canvas);
}

static void build_home(AppScreens *screens, AppScreensContext context)
{
    const Theme *theme = context.theme;
    screens->home_items[0] =
        app_screens_open_item(i18n_text(STR_RESCAN_MUSIC_TITLE), &screens->music.screen);
    screens->home_items[1] = shelf_item(screens, context, SPOKEN_AUDIOBOOK);
    screens->home_items[2] = shelf_item(screens, context, SPOKEN_PODCAST);
    screens->home_items[3] =
        app_screens_open_item(i18n_text(STR_MENU_SETTINGS), &screens->settings.menu.screen);
    screens->home_items[4] =
        app_screens_open_item(i18n_text(STR_DIAG_TITLE), &screens->diagnostics.screen);
    screens->home_items[5] =
        app_screens_open_item(i18n_text(STR_MENU_NOW_PLAYING), &screens->now_playing.screen);
    screens->home_items[5].shown = player_has_track;
    screens->home_items[5].shown_context = context.player;
    home_art_init(&screens->home_art, context.covers, context.store, context.player, theme);
    screens->home_art.notice = hal_storage_card_ready() ? NULL : i18n_text(STR_MENU_NO_CARD);
    screens->home_scene = (MenuScene){home_scene_update, home_scene_draw, screens, false};
    MenuConfig home = {.title = "EchoPod",
                       .items = screens->home_items,
                       .item_count = HOME_ITEM_COUNT,
                       .theme = theme,
                       .panel = {.width = HOME_ART_WIDTH,
                                 .min_height = HOME_ART_MIN_HEIGHT,
                                 .update = home_art_update,
                                 .draw = home_art_draw,
                                 .context = &screens->home_art},
                       .scene = &screens->home_scene};
    menu_screen_init(&screens->home, home);
}
#endif

ECHO_STARTUP_CODE Screen *app_screens_init(AppScreens *screens, AppScreensContext context)
{
    const Theme *theme = context.theme;
    bool card = hal_storage_card_ready();
    MessageConfig no_library = {
        .title = i18n_text(card ? STR_MENU_NO_LIBRARY : STR_MENU_NO_CARD),
        .message = i18n_text(card ? STR_MENU_NO_LIBRARY_HINT : STR_MENU_NO_CARD_HINT),
        .theme = theme};
    UpNextConfig up_next = {
        .library = &context.store->library, .theme = theme, .player = context.player};
    NowPlayingConfig now_playing = {.player = context.player,
                                    .library = &context.store->library,
                                    .theme = theme,
                                    .covers = context.covers,
                                    .up_next = up_next_screen_init(&screens->up_next, up_next),
                                    .options = context.settings->settings.options};
    EqualizerConfig equalizer = {
        .store = context.equalizer, .toast = context.toast, .theme = theme};
    now_playing.equalizer = equalizer_screen_init(&screens->equalizer, equalizer);
    message_screen_init(&screens->no_library, no_library);
    bluetooth_screen_init(&screens->bluetooth, context.bluetooth, theme);
    now_playing_screen_init(&screens->now_playing, now_playing);
    build_music(screens, context);
    build_settings(screens, context);
    diagnostics_menu_build(screens, context.options, theme);
    build_home(screens, context);
    return &screens->home.screen;
}

static int music_path(AppScreens *screens, int entry, Screen **path)
{
    BrowseEntry *browse = &screens->browse_entries[entry];
    if (!browse->store->available) {
        return 0;
    }
    list_view_select(&screens->music.list, music_position(entry), 0);
    path[0] = &screens->music.screen;
    path[1] = browse_screen_for(browse);
    return path[1] != NULL ? START_PATH_CAPACITY : 1;
}

int app_screens_start_path(AppScreens *screens, StartScreen start, Screen **path)
{
    if (start == START_NOW_PLAYING) {
        list_view_select(&screens->home.list, 1, 0);
        path[0] = &screens->now_playing.screen;
        return 1;
    }
#ifdef ECHO_LITE
    if (start == START_ALBUMS || start == START_ARTISTS || start == START_FAVORITES ||
        start == START_COVER_FLOW || start == START_ALBUM_ARTISTS || start == START_GENRES) {
        return 0;
    }
#endif
    if ((int)start >= (int)START_SCREEN_COUNT || START_ENTRIES[start] == START_HOME_ONLY) {
        return 0;
    }
    return music_path(screens, START_ENTRIES[start], path);
}
