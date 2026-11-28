#include "favorite_track_source.h"

#ifdef ECHO_LITE

#include "hal/audio.h"
#include "library/library.h"

enum {
    TRACKS_PER_AUDIO_SERVICE = 16
};

static uint32_t favorite_revision(void *context)
{
    FavoriteTrackSource *source = context;
    return source->favorites->revision;
}

static uint16_t favorite_collect(void *context, uint16_t *tracks, uint16_t capacity)
{
    FavoriteTrackSource *source = context;
    FavoritesStore *favorites = source->favorites;
    uint16_t found = 0;
    if (favorites->favorites.count == 0) {
        return 0;
    }
    uint16_t wanted = favorites->favorites.count > capacity ? capacity
                                                            : favorites->favorites.count;
    const Library *library = favorites->library;
    uint32_t total = library_count(library, LIBRARY_TITLE_ORDER);
    for (uint32_t position = 0; position < total && found < wanted; position++) {
        if (position % TRACKS_PER_AUDIO_SERVICE == 0) {
            hal_audio_service();
        }
        uint16_t track = 0;
        if (library_read_index(library, LIBRARY_TITLE_ORDER, position, &track) &&
            favorites_store_contains_track(favorites, track)) {
            tracks[found++] = track;
        }
    }
    return found;
}

static ScreenAction favorite_hold(void *context, uint16_t track, uint32_t now_ms)
{
    FavoriteTrackSource *source = context;
    FavoriteResult result = favorites_store_toggle_track(source->favorites, track);
    if (source->toast != NULL) {
        toast_show(source->toast, favorites_store_message(result), now_ms);
    }
    return screen_action_none();
}

TrackListSource favorite_track_source(FavoriteTrackSource *context, FavoritesStore *favorites,
                                      Toast *toast)
{
    *context = (FavoriteTrackSource){.favorites = favorites, .toast = toast};
    return (TrackListSource){.context = context,
                             .revision = favorite_revision,
                             .collect = favorite_collect,
                             .hold = favorite_hold};
}

#endif
