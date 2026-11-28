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
void lite_card_rounded(Canvas *canvas, const Theme *theme, Rect card, int shadow, int radius);
void lite_status_block_draw(Canvas *canvas, Point origin, Color color);
void lite_status_root_draw(Canvas *canvas, const Theme *theme, Point origin, Color all,
                            Color favorites);
Rect lite_list_frame(Canvas *canvas, const Theme *theme, const char *title);
Rect lite_list_frame_no_title(Canvas *canvas, const Theme *theme);
#endif
