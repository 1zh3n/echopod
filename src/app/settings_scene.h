#pragma once

#ifdef ECHO_LITE

#include "ui/menu_screen.h"

struct SettingsScreens;

MenuScene *settings_home_scene(struct SettingsScreens *screens);
MenuScene *settings_scene(struct SettingsScreens *screens);

#endif
