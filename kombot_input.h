#pragma once

#include "Windows.h"
#include "kombot_reftypes.h"

#define KOMBOT_ACTIVATION_KEY '9'
#define KOMBOT_ACTIVATION_MOUSE WM_LBUTTONDOWN
#define KOMBOT_DEACTIVATION_MOUSE WM_LBUTTONUP

void kombot_input_init(void);

void kombot_input_free(KOMBOT_PTR(void) pstate);
