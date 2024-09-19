#pragma once

#include "kombot_reftypes.h"

#define KOMBOT_AIM_SCREEN_DELTA_PXL 120

void kombot_aim_init(void);

void kombot_aim_start(void);

void kombot_aim_end(void);

void kombot_aim_free(KOMBOT_PTR(void) pstate);
