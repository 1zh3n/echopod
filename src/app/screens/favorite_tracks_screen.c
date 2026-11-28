#ifdef ECHO_LITE
#include "screens/favorite_tracks_screen.h"

#include <stdbool.h>

#include "core/i18n.h"
#include "hal/audio.h"
#include "ui/label.h"
#include "ui/lite_frame.h"
#include "ui/list_navigation.h"

enum {
    DETAIL_MAX_WIDTH = 110,
    TRACKS_PER_AUDIO_SERVICE = 16
};

static int min_of(int first, int second)
{
    return first < second ? first : second;
}

static void forget_rows(FavoriteTracksScreen *favorites)
{
    favorites->rows_first = -1;
    favorites->rows_count = 0;
}

static void collect_tracks(FavoriteTracksScreen *favorites)
{
    const Library *library = favorites->config.library;
    FavoritesStore *store = favorites->config.favorites;
    uint16_t found = 0;
    if (store->favorites.count > 0) {
        uint32_t total = library_count(library, LIBRARY_TITLE_ORDER);
        for (uint32_t position = 0; position < total; position++) {
            if (position % TRACKS_PER_AUDIO_SERVICE == 0) {
                hal_audio_service();
            }
            uint16_t track = 0;
            if (library_read_index(library, LIBRARY_TITLE_ORDER, position, &track) &&
                favorites_store_contains_track(store, track)) {
                favorites->tracks[found++] = track;
                if (found == PLAY_QUEUE_TRACK_LIST_MAX) {
                    break;
                }
            }
        }
    }
    favorites->count = found;
    favorites->revision = store->revision;
    if (favorites->list.selected >= favorites->count) {
        favorites->list.selected = favorites->count > 0 ? favorites->count - 1 : 0;
    }
    forget_rows(favorites);
}

static void load_row(FavoriteTracksScreen *favorites, int row, int index)
{
    const Library *library = favorites->config.library;
    char *label = favorites->labels[row];
    char *detail = favorites->details[row];
    label[0] = '\0';
    detail[0] = '\0';
    LibraryTrack track;
    if (index < 0 || index >= favorites->count ||
        !library_read_track(library, favorites->tracks[index], &track)) {
        return;
    }
    library_read_string(library, track.title, label, FAVORITE_TRACK_LABEL_CAPACITY);
    LibraryGroup artist;
    if (library_read_group(library, LIBRARY_ARTISTS, track.artist, &artist)) {
        library_read_string(library, artist.name, detail, FAVORITE_TRACK_DETAIL_CAPACITY);
    }
}

static void refresh_rows(FavoriteTracksScreen *favorites)
{
    int top = list_view_top(&favorites->list);
    int count = min_of(min_of(list_view_rows(&favorites->list), FAVORITE_TRACK_ROWS),
                       favorites->count - top);
    if (favorites->rows_first == top && favorites->rows_count >= count) {
        return;
    }
    for (int row = 0; row < count; row++) {
        load_row(favorites, row, top + row);
    }
    favorites->rows_first = top;
    favorites->rows_count = count;
}

static const char *cached_text(const FavoriteTracksScreen *favorites, int index, bool detail)
{
    int row = index - favorites->rows_first;
    if (favorites->rows_first < 0 || row < 0 || row >= favorites->rows_count) {
        return "";
    }
    return detail ? favorites->details[row] : favorites->labels[row];
}

static int row_count(const void *context)
{
    const FavoriteTracksScreen *favorites = context;
    return favorites->count;
}

static const char *row_label(const void *context, int index)
{
    return cached_text(context, index, false);
}

static const char *row_detail(const void *context, int index)
{
    const char *detail = cached_text(context, index, true);
    return detail[0] != '\0' ? detail : NULL;
}

static ScreenAction select_row(void *context, int index)
{
    FavoriteTracksScreen *favorites = context;
    if (index < 0 || index >= favorites->count ||
        !player_play_tracks(favorites->config.player, favorites->tracks,
                            (uint16_t)favorites->count, (uint16_t)index)) {
        return screen_action_none();
    }
    return screen_action_push(favorites->config.now_playing);
}

static ScreenAction remove_row(void *context, int index)
{
    FavoriteTracksScreen *favorites = context;
    if (index < 0 || index >= favorites->count) {
        return screen_action_none();
    }
    FavoriteResult result =
        favorites_store_toggle_track(favorites->config.favorites, favorites->tracks[index]);
    if (favorites->config.toast != NULL) {
        toast_show(favorites->config.toast, favorites_store_message(result), favorites->now_ms);
    }
    collect_tracks(favorites);
    refresh_rows(favorites);
    return screen_action_none();
}

static ScreenAction handle_key(void *context, KeyEvent event, uint32_t now_ms)
{
    FavoriteTracksScreen *favorites = context;
    favorites->now_ms = now_ms;
    ListSelection selection = {
        .select = select_row, .hold = remove_row, .context = favorites};
    ScreenAction action = list_navigation_handle(&favorites->list, event, now_ms, selection);
    refresh_rows(favorites);
    return action;
}

static void set_visible_rows(FavoriteTracksScreen *favorites)
{
    favorites->list.visible_rows = lite_list_rows(favorites->config.theme);
}

static void enter(void *context, uint32_t now_ms)
{
    FavoriteTracksScreen *favorites = context;
    favorites->now_ms = now_ms;
    collect_tracks(favorites);
    set_visible_rows(favorites);
    list_view_restart_marquee(&favorites->list, now_ms);
    refresh_rows(favorites);
}

static void update(void *context, uint32_t now_ms)
{
    FavoriteTracksScreen *favorites = context;
    favorites->now_ms = now_ms;
    if (favorites->revision != favorites->config.favorites->revision) {
        collect_tracks(favorites);
    }
    set_visible_rows(favorites);
    refresh_rows(favorites);
}

static void draw(void *context, Canvas *canvas)
{
    FavoriteTracksScreen *favorites = context;
    const Theme *theme = favorites->config.theme;
    Rect content = lite_list_frame_no_title(canvas, theme);
    if (favorites->count == 0) {
        TextStyle style = {.font = theme->fonts.body, .color = theme->colors->text_secondary};
        const char *message = i18n_text(STR_MENU_NO_SONG_FAVORITES);
        Rect area = label_centered_area(style.font, content, message);
        label_draw(canvas, style, (Label){.area = area, .text = message});
        return;
    }
    list_view_draw_over(&favorites->list, canvas, content, favorites->now_ms);
}

Screen *favorite_tracks_screen_init(FavoriteTracksScreen *favorites, FavoriteTracksConfig config)
{
    favorites->config = config;
    favorites->count = 0;
    favorites->rows_first = -1;
    favorites->rows_count = 0;
    favorites->revision = 0;
    favorites->now_ms = 0;
    favorites->model = (ListModel){.context = favorites,
                                   .count = row_count,
                                   .label = row_label,
                                   .detail = row_detail,
                                   .detail_max_width = DETAIL_MAX_WIDTH};
    list_view_init(&favorites->list, &favorites->model, config.theme, 0);
    favorites->screen = (Screen){
        .title = i18n_text(STR_MENU_FAVORITES),
        .context = favorites,
        .overlay = true,
        .enter = enter,
        .handle_key = handle_key,
        .update = update,
        .draw = draw,
    };
    return &favorites->screen;
}
#endif
