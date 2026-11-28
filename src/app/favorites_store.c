#include "favorites_store.h"
#include "core/resident.h"

#include "core/i18n.h"
#include "hal/storage.h"

const char FAVORITES_STORE_PATH[] = "/.echo/favorites.bin";
const char FAVORITE_TRACKS_PATH[] = "/.echo/favorite_tracks.bin";

static bool read_favorites(const char *path, uint8_t *bytes, uint32_t *length)
{
    HalFile file;
    if (!hal_file_open(&file, path)) {
        return false;
    }
    *length = file.size < FAVORITES_FILE_CAPACITY ? file.size : FAVORITES_FILE_CAPACITY;
    bool read = hal_file_read(file, 0, bytes, *length);
    hal_file_close(&file);
    return read;
}

static void load_favorites(FavoritesStore *store, const Library *library, const char *path)
{
    uint8_t bytes[FAVORITES_FILE_CAPACITY];
    uint32_t length = 0;
    store->library = library;
    store->revision++;
    if (!read_favorites(path, bytes, &length) ||
        !favorites_decode(bytes, length, &store->favorites)) {
        favorites_clear(&store->favorites);
    }
}

ECHO_STARTUP_CODE void favorites_store_load(FavoritesStore *store, const Library *library)
{
    load_favorites(store, library, FAVORITES_STORE_PATH);
}

ECHO_STARTUP_CODE void favorites_store_load_tracks(FavoritesStore *store, const Library *library)
{
    load_favorites(store, library, FAVORITE_TRACKS_PATH);
}

static bool contains_key(const FavoritesStore *store, uint32_t key)
{
    return favorites_contains(&store->favorites, key);
}

bool favorites_store_contains_album(const FavoritesStore *store, uint16_t album)
{
    if (store->favorites.count == 0) {
        return false;
    }
    return contains_key(store, library_album_key(store->library, album));
}

bool favorites_store_contains_track(const FavoritesStore *store, uint16_t track)
{
    if (store->favorites.count == 0) {
        return false;
    }
    return contains_key(store, library_track_key(store->library, track));
}

static bool save(const FavoritesStore *store, const char *path)
{
    uint8_t bytes[FAVORITES_FILE_CAPACITY];
    uint32_t length = favorites_encode(&store->favorites, bytes);
    return hal_file_write(path, bytes, length);
}

static FavoriteResult toggle_key(FavoritesStore *store, uint32_t key, const char *path)
{
    bool present = favorites_contains(&store->favorites, key);
    if (!present && store->favorites.count == FAVORITES_CAPACITY) {
        return (FavoriteResult){.change = FAVORITE_FULL, .saved = true};
    }
    favorites_toggle(&store->favorites, key);
    store->revision++;
    FavoriteChange change = present ? FAVORITE_REMOVED : FAVORITE_ADDED;
    return (FavoriteResult){.change = change, .saved = save(store, path)};
}

FavoriteResult favorites_store_toggle_album(FavoritesStore *store, uint16_t album)
{
    return toggle_key(store, library_album_key(store->library, album), FAVORITES_STORE_PATH);
}

FavoriteResult favorites_store_toggle_track(FavoritesStore *store, uint16_t track)
{
    return toggle_key(store, library_track_key(store->library, track), FAVORITE_TRACKS_PATH);
}

const char *favorites_store_message(FavoriteResult result)
{
    if (!result.saved) {
        return i18n_text(STR_MENU_FAVORITES_SAVE_FAILED);
    }
    switch (result.change) {
    case FAVORITE_ADDED:
        return i18n_text(STR_MENU_FAVORITES_ADDED);
    case FAVORITE_REMOVED:
        return i18n_text(STR_MENU_FAVORITES_REMOVED);
    case FAVORITE_FULL:
        return i18n_text(STR_MENU_FAVORITES_FULL);
    }
    return "";
}
