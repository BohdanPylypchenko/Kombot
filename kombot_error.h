#pragma once

#include "Windows.h"
#include "kombot_reftypes.h"

typedef enum {
    KOMBOT_ERROR_NO_ERROR,
    KOMBOT_ERROR_INPUT_INIT
} kombot_error_type;

typedef struct {
    kombot_error_type error_type;
    DWORD error_code;
} kombot_error;

void kombot_error_set_global(
    kombot_error_type error_type,
    DWORD error_code
);

KOMBOT_PTR(kombot_error) kombot_error_get_global(void);
