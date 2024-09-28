#pragma once

#include "Windows.h"
#include "kombot_reftypes.h"

#define KOMBOT_ACTIVATION_KEY_REGULAR '9'
#define KOMBOT_ACTIVATION_KEY_SYSTEM VK_LMENU

void kombot_input_init(void);

void kombot_input_free(KOMBOT_PTR(void) pstate);
