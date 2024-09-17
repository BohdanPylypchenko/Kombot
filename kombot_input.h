#pragma once

#include "Windows.h"
#include "kombot_reftypes.h"

#define KOMBOT_ACTIVATION_KEY '9'
#define KOMBOT_ACTIVATION_MOUSE WM_LBUTTONDOWN

void kombot_input_state_init(void);

void kombot_input_state_free(KOMBOT_PTR(void) pstate);
