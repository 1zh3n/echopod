#include "screens/track_list_screen.h"

#ifdef ECHO_LITE

#include <stdbool.h>

#include "core/format.h"
#include "core/i18n.h"
#include "library/sort_key.h"
#include "ui/label.h"
#include "ui/lite_frame.h"
#include "ui/list_navigation.h"

enum {
    DETAIL_MAX_WIDTH = 110
};

static int min_of(int first, int second)
{
    return first < second ? first : second;
}

static void forget_rows(TrackListScreen *screen)
{
    screen->rows_first = -1;
    screen->rows_count = 0;
}

static void collect_tracks(TrackListScreen *screen)
{
    screen->count = screen->config.source.collect(screen->config.source.context, screen->tracks,
                                                  PLAY_QUEUE_TRACK_LIST_MAX);
    screen->revision = screen->config.source.revision(screen->config.source.context);
    screen->library_stamp = library_fingerprint(screen->config.library);
    if (screen->list.selected >= screen->count) {
        screen->list.selected = screen->count > 0 ? screen->count - 1 : 0;
    }
    forget_rows(screen);
}

static bool source_changed(const TrackListScreen *screen)
{
    return screen->revision != screen->config.source.revision(screen->config.source.context) ||
           screen->library_stamp != library_fingerprint(screen->config.library);
}

static void load_row(TrackListScreen *screen, int row, int index)
{
    const Library *library = screen->config.library;
    char *label = screen->labels[row];
    char *detail = screen->details[row];
    label[0] = '\0';
    detail[0] = '\0';
    LibraryTrack track;
    if (index < 0 || index >= screen->count ||
        !library_read_track(library, screen->tracks[index], &track)) {
        return;
    }
    library_read_string(library, track.title, label, TRACK_LIST_LABEL_CAPACITY);
    format_duration(track.duration_ms, detail, TRACK_LIST_DETAIL_CAPACITY);
}

static void refresh_rows(TrackListScreen *screen)
{
    int top = list_view_top(&screen->list);
    int count =
        min_of(min_of(list_view_rows(&screen->list), TRACK_LIST_ROWS), screen->count - top);
    if (screen->rows_first == top && screen->rows_count >= count) {
        return;
    }
    for (int row = 0; row < count; row++) {
        load_row(screen, row, top + row);
    }
    screen->rows_first = top;
    screen->rows_count = count;
}

static int row_count(const void *context)
{
    const TrackListScreen *screen = context;
    return screen->count;
}

static const char *cached_text(const TrackListScreen *screen, int index, bool detail)
{
    int row = index - screen->rows_first;
    if (screen->rows_first < 0 || row < 0 || row >= screen->rows_count) {
        return "";
    }
    return detail ? screen->details[row] : screen->labels[row];
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

static int row_section(const void *context, int index)
{
    const TrackListScreen *screen = context;
    char label[TRACK_LIST_LABEL_CAPACITY];
    LibraryTrack track;
    label[0] = '\0';
    if (index >= 0 && index < screen->count &&
        library_read_track(screen->config.library, screen->tracks[index], &track)) {
        library_read_string(screen->config.library, track.title, label, sizeof label);
    }
    char initial[2] = {sort_key_initial(label), '\0'};
    return list_section_of_key(initial);
}

static ScreenAction select_row(void *context, int index)
{
    TrackListScreen *screen = context;
    if (index < 0 || index >= screen->count ||
        !player_play_tracks(screen->config.player, screen->tracks, (uint16_t)screen->count,
                            (uint16_t)index)) {
        return screen_action_none();
    }
    return screen_action_push(screen->config.now_playing);
}

static ScreenAction hold_row(void *context, int index)
{
    TrackListScreen *screen = context;
    if (index < 0 || index >= screen->count || screen->config.source.hold == NULL) {
        return screen_action_none();
    }
    ScreenAction action =
        screen->config.source.hold(screen->config.source.context, screen->tracks[index],
                                   screen->now_ms);
    if (source_changed(screen)) {
        collect_tracks(screen);
        refresh_rows(screen);
    }
    return action;
}

static void set_visible_rows(TrackListScreen *screen)
{
    screen->list.visible_rows = lite_list_rows(screen->config.theme);
}

static void update_sections(TrackListScreen *screen)
{
    screen->screen.long_menu_enabled = list_view_sections_vary(&screen->list);
}

static ScreenAction open_section_jump(void *context, uint32_t now_ms)
{
    (void)now_ms;
    TrackListScreen *screen = context;
    SectionJumpConfig config = {.list = &screen->list, .theme = screen->config.theme};
    return screen_action_push(section_jump_screen_init(&screen->section_jump, config));
}

static void enter(void *context, uint32_t now_ms)
{
    TrackListScreen *screen = context;
    screen->now_ms = now_ms;
    if (source_changed(screen)) {
        collect_tracks(screen);
    }
    set_visible_rows(screen);
    list_view_restart_marquee(&screen->list, now_ms);
    refresh_rows(screen);
    update_sections(screen);
}

static void update(void *context, uint32_t now_ms)
{
    TrackListScreen *screen = context;
    screen->now_ms = now_ms;
    if (source_changed(screen)) {
        collect_tracks(screen);
    }
    set_visible_rows(screen);
    refresh_rows(screen);
    update_sections(screen);
}

static ScreenAction handle_key(void *context, KeyEvent event, uint32_t now_ms)
{
    TrackListScreen *screen = context;
    screen->now_ms = now_ms;
    ListSelection selection = {
        .select = select_row, .hold = hold_row, .context = screen};
    ScreenAction action = list_navigation_handle(&screen->list, event, now_ms, selection);
    refresh_rows(screen);
    update_sections(screen);
    return action;
}

static void draw(void *context, Canvas *canvas)
{
    TrackListScreen *screen = context;
    const Theme *theme = screen->config.theme;
    Rect content = lite_list_frame_no_title(canvas, theme);
    if (screen->count == 0) {
        TextStyle style = {.font = theme->fonts.body, .color = theme->colors->text_secondary};
        const char *message = i18n_text(screen->config.empty_message);
        Rect area = label_centered_area(style.font, content, message);
        label_draw(canvas, style, (Label){.area = area, .text = message});
        return;
    }
    list_view_draw_over(&screen->list, canvas, content, screen->now_ms);
}

Screen *track_list_screen_init(TrackListScreen *screen, TrackListConfig config)
{
    screen->config = config;
    screen->count = 0;
    screen->rows_first = -1;
    screen->rows_count = 0;
    screen->revision = 0;
    screen->library_stamp = 0;
    screen->now_ms = 0;
    screen->model = (ListModel){.context = screen,
                                .count = row_count,
                                .label = row_label,
                                .detail = row_detail,
                                .section = row_section,
                                .detail_max_width = DETAIL_MAX_WIDTH};
    list_view_init(&screen->list, &screen->model, config.theme, 0);
    screen->screen = (Screen){
        .title = i18n_text(config.title),
        .context = screen,
        .overlay = true,
        .long_menu_enabled = false,
        .long_menu_context = screen,
        .long_menu = open_section_jump,
        .enter = enter,
        .handle_key = handle_key,
        .update = update,
        .draw = draw,
    };
    return &screen->screen;
}

#endif
