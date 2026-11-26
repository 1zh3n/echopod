#pragma once

#include <stdbool.h>

#include "gfx/color.h"

void lite_status_set(Color shadow, Color progress, bool bluetooth);
Color lite_status_shadow(void);
Color lite_status_progress(void);
bool lite_status_bluetooth(void);
