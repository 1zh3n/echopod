#pragma once

#ifdef ECHO_LITE

#include <stdint.h>

#include "core/strings.h"
#include "library/library.h"
#include "player.h"
#include "screens/section_jump_screen.h"
#include "ui/list_view.h"
#include "ui/screen.h"
#include "ui/theme.h"

enum {
    TRACK_LIST_ROWS = 9,
    TRACK_LIST_LABEL_CAPACITY = 128,
    TRACK_LIST_DETAIL_CAPACITY = 64
};

typedef struct TrackListSource {
    void *context;
    uint32_t (*revision)(void *context);
    uint16_t (*collect)(void *context, uint16_t *tracks, uint16_t capacity);
    ScreenAction (*hold)(void *context, uint16_t track, uint32_t now_ms);
} TrackListSource;

typedef struct TrackListConfig {
    const Library *library;
    Player *player;
    Screen *now_playing;
    const Theme *theme;
    TrackListSource source;
    StringId title;
    StringId empty_message;
} TrackListConfig;

typedef struct TrackListScreen {
    Screen screen;
    TrackListConfig config;
    ListModel model;
    ListView list;
    SectionJumpScreen section_jump;
    int count;
    int rows_first;
    int rows_count;
    uint16_t tracks[PLAY_QUEUE_TRACK_LIST_MAX];
    char labels[TRACK_LIST_ROWS][TRACK_LIST_LABEL_CAPACITY];
    char details[TRACK_LIST_ROWS][TRACK_LIST_DETAIL_CAPACITY];
    uint32_t revision;
    uint32_t library_stamp;
    uint32_t now_ms;
} TrackListScreen;

Screen *track_list_screen_init(TrackListScreen *screen, TrackListConfig config);

#endif
