#pragma once

#ifdef ECHO_LITE

#include <stdbool.h>
#include <stdint.h>

#include "gfx/canvas.h"

void lite_animation_reset(void);
bool lite_animation_ready(void);
bool lite_animation_load(void);
uint8_t lite_animation_frames(void);
void lite_animation_draw_card(Canvas *canvas, Rect card, Color shadow, Color ink, Color panel,
                              uint32_t now_ms);

#endif
