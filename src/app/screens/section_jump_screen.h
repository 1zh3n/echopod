#pragma once

#ifdef ECHO_LITE

#include <stdint.h>

#include "ui/list_view.h"
#include "ui/screen.h"
#include "ui/theme.h"

typedef struct SectionJumpConfig {
    ListView *list;
    const Theme *theme;
} SectionJumpConfig;

typedef struct SectionJumpScreen {
    Screen screen;
    ListView *list;
    const Theme *theme;
    uint8_t selected;
    uint32_t now_ms;
} SectionJumpScreen;

Screen *section_jump_screen_init(SectionJumpScreen *jump, SectionJumpConfig config);

#endif
