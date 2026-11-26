#include "ui/lite_status.h"

static Color shadow_color;
static Color progress_color;
static bool bluetooth_connected;
static bool status_ready;

void lite_status_set(Color shadow, Color progress, bool bluetooth)
{
    shadow_color = shadow;
    progress_color = progress;
    bluetooth_connected = bluetooth;
    status_ready = true;
}

Color lite_status_shadow(void)
{
    return status_ready ? shadow_color : COLOR_RGB(10, 10, 10);
}

Color lite_status_progress(void)
{
    return status_ready ? progress_color : COLOR_RGB(0, 111, 238);
}

bool lite_status_bluetooth(void)
{
    return bluetooth_connected;
}
