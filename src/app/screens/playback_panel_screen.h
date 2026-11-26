#pragma once

#ifdef ECHO_LITE

#include <stdint.h>

#include "ui/screen.h"
#include "ui/theme.h"

typedef struct SettingsContext SettingsContext;

typedef struct PlaybackPanelScreen {
    Screen screen;
    SettingsContext *settings;
    const Theme *theme;
    uint8_t selected;
    uint32_t now_ms;
} PlaybackPanelScreen;

Screen *playback_panel_screen_init(PlaybackPanelScreen *panel, SettingsContext *settings,
                                   const Theme *theme);

#endif
