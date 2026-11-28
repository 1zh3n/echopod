#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal/system.h"

typedef struct Asset {
    uint8_t *bytes;
    uint32_t size;
    bool loaded;
} Asset;

static Asset animation;
static Asset icons;

static void load(Asset *asset, const char *path)
{
    if (asset->loaded) {
        return;
    }
    asset->loaded = true;
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return;
    }
    long size = fseek(file, 0, SEEK_END) == 0 ? ftell(file) : -1;
    uint8_t *bytes = size > 0 ? malloc((size_t)size) : NULL;
    bool read = bytes != NULL && fseek(file, 0, SEEK_SET) == 0 &&
                fread(bytes, 1, (size_t)size, file) == (size_t)size;
    fclose(file);
    if (!read) {
        free(bytes);
        return;
    }
    *asset = (Asset){.bytes = bytes, .size = (uint32_t)size, .loaded = true};
}

static bool read(const Asset *asset, uint32_t offset, void *target, uint32_t length)
{
    if (asset->bytes == NULL || offset > asset->size || length > asset->size - offset) {
        return false;
    }
    memcpy(target, asset->bytes + offset, length);
    return true;
}

void *hal_system_lite_animation(uint32_t *size)
{
    static uint32_t memory[HAL_LITE_ANIMATION_BYTES / sizeof(uint32_t)];
    *size = sizeof memory;
    return memory;
}

uint32_t hal_system_lite_animation_size(void)
{
    load(&animation, ECHO_LITE_ANIMATION_ASSET);
    return animation.size;
}

bool hal_system_lite_animation_read(uint32_t offset, void *target, uint32_t length)
{
    load(&animation, ECHO_LITE_ANIMATION_ASSET);
    return read(&animation, offset, target, length);
}

uint32_t hal_system_lite_icons_size(void)
{
    load(&icons, ECHO_LITE_ICONS_ASSET);
    return icons.size;
}

bool hal_system_lite_icons_read(uint32_t offset, void *target, uint32_t length)
{
    load(&icons, ECHO_LITE_ICONS_ASSET);
    return read(&icons, offset, target, length);
}
