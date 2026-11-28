#pragma once

#include <stdint.h>

enum {
    VOLUME_LEGACY_MAX = 50,
    VOLUME_LEVEL_MAX = 85
};

uint8_t volume_level_from_legacy(uint8_t level);
