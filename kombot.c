#include "pch.h"

#include "kombot_input.h"
#include "kombot_error.h"

typedef enum {
    KOMBOT_RUN,
    KOMBOT_END_FAIL_INPUT_INIT
} kombot_execution_state;

int main(void) {
    kombot_execution_state execution_state = KOMBOT_RUN;

    kombot_input_state input_state;

    kombot_input_state_init(&input_state);
    if (kombot_error_get_global()->error_type == KOMBOT_ERROR_INPUT_INIT) {
        execution_state = KOMBOT_END_FAIL_INPUT_INIT;
        goto final;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

final:
    switch (execution_state) {
        case KOMBOT_END_FAIL_INPUT_INIT:
            fprintf(
                stderr,
                "Error: failed to initialize keyboard/mouse hooks, code = %d\n",
                kombot_error_get_global()->error_code
            );
            return 1;
        default:
            fprintf(
                stderr,
                "Error: corrupred kombot execution state\n"
            );
            return 1;
    }
}