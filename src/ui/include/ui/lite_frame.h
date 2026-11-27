#pragma once

#include <stdbool.h>

#include "gfx/canvas.h"
#include "ui/theme.h"

#ifdef ECHO_LITE
bool lite_theme_is_dark(const Theme *theme);
Color lite_card_border_color(const Theme *theme);
int lite_card_border_width(const Theme *theme);
Color lite_card_shadow_color(const Theme *theme);
int lite_list_rows(const Theme *theme);
Rect lite_card(Canvas *canvas, const Theme *theme, Rect card, int shadow);
Rect lite_list_frame(Canvas *canvas, const Theme *theme, const char *title);
#endif
