#include "library_store.h"

#include <stddef.h>

const char LIBRARY_STORE_PATH[] = "/.echo/library.db";
#ifdef ECHO_LITE
const char PRIVATE_LIBRARY_STORE_PATH[] = "/.echo/private.db";
#endif

static bool read_file(void *context, uint32_t offset, void *buffer, uint32_t length)
{
    const LibraryStore *store = context;
    return hal_file_read(store->file, offset, buffer, length);
}

void library_store_close(LibraryStore *store)
{
    if (store->available) {
        hal_file_close(&store->file);
    }
    store->available = false;
}

bool library_store_open(LibraryStore *store)
{
    return library_store_open_path(store, LIBRARY_STORE_PATH);
}

bool library_store_open_path(LibraryStore *store, const char *path)
{
    library_store_close(store);
    store->library = (Library){.source = {.read = NULL, .context = NULL}};
    if (!hal_file_open(&store->file, path)) {
        return false;
    }
    ByteSource file_source = {.read = read_file, .context = store};
    block_cache_init(&store->cache, file_source, store->file.size);
    LibraryStatus status =
        library_open(&store->library, block_cache_source(&store->cache), store->file.size);
    store->available = status == LIBRARY_OK;
    if (!store->available) {
        hal_file_close(&store->file);
    }
    return store->available;
}
