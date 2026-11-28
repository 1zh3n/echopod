#pragma once

#include "favorites_store.h"
#include "screens/track_list_screen.h"
#include "ui/toast.h"

#ifdef ECHO_LITE
typedef struct FavoriteTrackSource {
    FavoritesStore *favorites;
    Toast *toast;
} FavoriteTrackSource;

TrackListSource favorite_track_source(FavoriteTrackSource *context, FavoritesStore *favorites,
                                      Toast *toast);
#endif
