#include "pch.h"

#include "kombot_error.h"
#include "kombot_reftypes.h"

static kombot_error global_error = {
    .error_type = KOMBOT_ERROR_NO_ERROR,
    .error_code = 0
};

void kombot_error_set_global(
    kombot_error_type error_type,
    DWORD error_code
) {
    global_error.error_type = error_type;
    global_error.error_code = error_code;
}

KOMBOT_PTR(kombot_error) kombot_error_get_global(void) {
    return &global_error;
}
