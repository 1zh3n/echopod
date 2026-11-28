#include "private_track_source.h"

#ifdef ECHO_LITE

#include "hal/audio.h"
#include "library/library.h"

enum {
    TRACKS_PER_AUDIO_SERVICE = 16
};

static uint32_t private_revision(void *context)
{
    PrivateTrackSource *source = context;
    return library_fingerprint(source->library);
}

static uint16_t private_collect(void *context, uint16_t *tracks, uint16_t capacity)
{
    PrivateTrackSource *source = context;
    uint32_t total = library_count(source->library, LIBRARY_TITLE_ORDER);
    uint16_t found = 0;
    uint16_t wanted = total > capacity ? capacity : (uint16_t)total;
    for (uint32_t position = 0; position < total && found < wanted; position++) {
        if (position % TRACKS_PER_AUDIO_SERVICE == 0) {
            hal_audio_service();
        }
        uint16_t track = 0;
        if (library_read_index(source->library, LIBRARY_TITLE_ORDER, position, &track)) {
            tracks[found++] = track;
        }
    }
    return found;
}

TrackListSource private_track_source(PrivateTrackSource *context, const Library *library)
{
    *context = (PrivateTrackSource){.library = library};
    return (TrackListSource){
        .context = context, .revision = private_revision, .collect = private_collect, .hold = NULL};
}

#endif
