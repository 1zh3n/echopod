#pragma once

#include <stddef.h>
#include <stdint.h>

#include "gfx/canvas.h"
#include "hal/power.h"
#include "ui/theme.h"

void charge_screen_draw(Canvas *canvas, const Theme *theme, HalBattery battery, uint32_t now_ms);
void charge_screen_text(HalBattery battery, char *text, size_t capacity);
