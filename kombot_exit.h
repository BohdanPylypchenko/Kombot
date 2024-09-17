#pragma once

typedef enum {
    KOMBOT_SUCCESS = 0,
    KOMBOT_FAIL    = 1
} KOMBOT_EXIT_CODE;

void kombot_user_exit_handler_init(void);
