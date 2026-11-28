#include "core/volume_curve.h"

uint8_t volume_level_from_legacy(uint8_t level)
{
    if (level <= 1) {
        return level;
    }
    if (level == 2) {
        return 8;
    }
    if (level == 3) {
        return 19;
    }
    uint32_t mapped = 25u + ((uint32_t)level * 6u + 2u) / 5u;
    return mapped > VOLUME_LEVEL_MAX ? VOLUME_LEVEL_MAX : (uint8_t)mapped;
}
