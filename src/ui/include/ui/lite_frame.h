#pragma once

#include "gfx/canvas.h"
#include "ui/theme.h"

#ifdef ECHO_LITE
Rect lite_list_frame(Canvas *canvas, const Theme *theme, const char *title);
#endif
