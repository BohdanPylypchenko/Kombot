#pragma once

#include "Windows.h"
#include "kombot_reftypes.h"

#define KOMBOT_ACTIVATION_KEY '9'

typedef struct {
    HHOOK keyboard_hook;
    HHOOK mouse_hook;
} kombot_input_state;

KOMBOT_PTR(kombot_input_state) kombot_input_state_init(
    KOMBOT_CONSTREF_RPTR(kombot_input_state) state
);

void kombot_input_state_free(
    KOMBOT_CONST_RPTR(kombot_input_state) state
);
