#pragma once

#ifdef ECHO_LITE

#include "library/library.h"
#include "screens/track_list_screen.h"

typedef struct PrivateTrackSource {
    const Library *library;
} PrivateTrackSource;

TrackListSource private_track_source(PrivateTrackSource *context, const Library *library);

#endif
