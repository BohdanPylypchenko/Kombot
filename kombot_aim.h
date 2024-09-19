#pragma once

#include "kombot_reftypes.h"

#define KOMBOT_SCREEN_HPROP0RTION 16
#define KOMBOT_SCREEN_VPROP0RTION 10

void kombot_aim_init(void);

void kombot_aim_start(void);

void kombot_aim_end(void);

void kombot_aim_free(KOMBOT_PTR(void) pstate);
