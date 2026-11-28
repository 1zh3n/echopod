#pragma once

#include <stdbool.h>

#include "hal/storage.h"
#include "library/block_cache.h"
#include "library/library.h"

typedef struct LibraryStore {
    HalFile file;
    BlockCache cache;
    Library library;
    bool available;
} LibraryStore;

extern const char LIBRARY_STORE_PATH[];
#ifdef ECHO_LITE
extern const char PRIVATE_LIBRARY_STORE_PATH[];
#endif

bool library_store_open(LibraryStore *store);
bool library_store_open_path(LibraryStore *store, const char *path);
void library_store_close(LibraryStore *store);
