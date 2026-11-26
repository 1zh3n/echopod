#pragma once

#include <stdint.h>

#include "core/i18n.h"
#include "core/power_policy.h"
#include "player.h"
#include "screens/bluetooth_screen.h"
#include "screens/date_time_screen.h"
#include "screens/equalizer_screen.h"
#include "screens/folders_screen.h"
#include "settings_store.h"
#include "ui/choice_screen.h"
#include "ui/menu_screen.h"
#include "ui/theme.h"
#include "ui/toast.h"

typedef enum SettingsGroup {
    SETTINGS_SOUND,
    SETTINGS_DISPLAY,
    SETTINGS_LIBRARY,
    SETTINGS_BUTTONS,
    SETTINGS_SYSTEM,
    SETTINGS_GROUP_COUNT
} SettingsGroup;

enum {
    SETTINGS_CHOICE_COUNT = 33,
    SETTINGS_TOP_COUNT = SETTINGS_GROUP_COUNT,
    SETTINGS_GROUP_ROWS_MAX = 10
};

typedef struct SettingsContext {
    SettingsStore *store;
    Player *player;
    PowerPolicy *power;
    Toast *toast;
    bool *rescan_requested;
    BluetoothScreen *bluetooth;
    EqualizerScreen *equalizer;
    DateTimeScreen *date_time;
    FoldersScreen *folders;
    int sleep_option;
} SettingsContext;

typedef struct SettingChoice {
    StringId title;
    const StringId *options;
    const char *const *plain_options;
    int option_count;
    int (*current)(const void *context);
    void (*choose)(void *context, int index);
    const char *(*describe)(int current);
    uint8_t slot;
} SettingChoice;

typedef struct SettingsScreens SettingsScreens;

typedef struct SettingsGroupLink {
    SettingsScreens *owner;
    uint8_t group;
} SettingsGroupLink;

typedef struct SettingsEntry {
    const SettingChoice *choice;
    SettingsScreens *owner;
} SettingsEntry;

struct SettingsScreens {
    SettingsContext context;
    const Theme *theme;
    MenuScreen menu;
    ChoiceScreen choice;
#ifndef ECHO_LITE
    MenuScreen group;
    SettingsGroupLink links[SETTINGS_GROUP_COUNT];
    MenuItem items[SETTINGS_TOP_COUNT];
#endif
    SettingsEntry entries[SETTINGS_CHOICE_COUNT];
    uint8_t selected[SETTINGS_GROUP_COUNT];
    uint8_t shown;
    MenuItem rows[SETTINGS_GROUP_ROWS_MAX];
};

Screen *settings_screens_init(SettingsScreens *screens, SettingsContext context,
                              const Theme *theme);
int settings_sleep_current(const SettingsContext *context);
const char *settings_sleep_label(int option);
void settings_sleep_apply(SettingsContext *context, int option, uint32_t now_ms);
void settings_playback_set_mode(SettingsContext *context, PlaybackMode mode, uint32_t seed);
#ifdef ECHO_LITE
const char *settings_category_title(int group);
#endif
