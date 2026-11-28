#pragma once

#include <stdint.h>

#include "favorites_store.h"
#include "library/library.h"
#include "player.h"
#include "ui/list_view.h"
#include "ui/screen.h"
#include "ui/theme.h"
#include "ui/toast.h"

enum {
    FAVORITE_TRACK_ROWS = 9,
    FAVORITE_TRACK_LABEL_CAPACITY = 128,
    FAVORITE_TRACK_DETAIL_CAPACITY = 64
};

typedef struct FavoriteTracksConfig {
    const Library *library;
    FavoritesStore *favorites;
    Player *player;
    Screen *now_playing;
    const Theme *theme;
    Toast *toast;
} FavoriteTracksConfig;

typedef struct FavoriteTracksScreen {
    Screen screen;
    FavoriteTracksConfig config;
    ListModel model;
    ListView list;
    int count;
    int rows_first;
    int rows_count;
    uint16_t tracks[PLAY_QUEUE_TRACK_LIST_MAX];
    char labels[FAVORITE_TRACK_ROWS][FAVORITE_TRACK_LABEL_CAPACITY];
    char details[FAVORITE_TRACK_ROWS][FAVORITE_TRACK_DETAIL_CAPACITY];
    uint32_t revision;
    uint32_t now_ms;
} FavoriteTracksScreen;

Screen *favorite_tracks_screen_init(FavoriteTracksScreen *favorites, FavoriteTracksConfig config);
