#pragma once

#include "kombot_reftypes.h"

#define KOMBOT_AIM_SCREEN_DELTA_PXL 120

#define KOMBOT_AIM_PIXEL_BLUE 0
#define KOMBOT_AIM_PIXEL_GREEN 0
#define KOMBOT_AIM_PIXEL_RED 254

void kombot_aim_init(void);

void kombot_aim_start(void);

void kombot_aim_end(void);

void kombot_aim_free(KOMBOT_PTR(void) pstate);
