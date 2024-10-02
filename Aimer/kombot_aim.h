#pragma once

#include "kombot_reftypes.h"

#define KOMBOT_AIM_SCREEN_DELTA_PXL 112
#define KOMBOT_AIM_SCREEN_RW 16
#define KOMBOT_AIM_SCREEN_RH 10

#define KOMBOT_AIM_PIXEL_BLUE 0
#define KOMBOT_AIM_PIXEL_GREEN 0
#define KOMBOT_AIM_PIXEL_RED 254

//#define KOMBOT_AIM_MOVE_PX_RD_X 300
//#define KOMBOT_AIM_MOVE_PX_RD_Y 300
//#define KOMBOT_AIM_MOVE_PX_RD_X 150
//#define KOMBOT_AIM_MOVE_PX_RD_Y 150
#define KOMBOT_AIM_MOVE_PX_RD_X 100
#define KOMBOT_AIM_MOVE_PX_RD_Y 100

#define KOMBOT_AIM_HORIZONTAL_FOV 128

void kombot_aim_init(void);

void kombot_aim_start(void);

void kombot_aim_end(void);

void kombot_aim_free(KOMBOT_PTR(void) pstate);
