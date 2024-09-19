#include "pch.h"

#include "kombot_exit.h"
#include "kombot_resource.h"
#include "kombot_exception.h"
#include "kombot_exit.h"

static DWORD exception_filter(DWORD ecode) {
    DWORD extracted_ecode = 0;
    KOMBOT_EXCEPTION_EXTRACT_ERROR_CODE(
        extracted_ecode,
        ecode
    );

    switch (extracted_ecode) {
    case KOMBOT_EXCEPTION_EXIT_HANDLER_INIT:
        fprintf(
            stderr,
            "Error: failed to initialize CtrlConsole event handler, "
            "exception code = %d; "
            "extracted exception code = %d"
            "\n", ecode, extracted_ecode
        );
        break;
    case KOMBOT_EXCEPTION_INPUT_INIT:
        fprintf(
            stderr,
            "Error: failed to initialize keyboard/mouse hooks, "
            "exception code = %d; "
            "extracted exception code = %d"
            "\n", ecode, extracted_ecode
        );
        break;
    case KOMBOT_EXCEPTION_AIM_DELTA_FAIL:
        fprintf(
            stderr,
            "Error: horizontal screen delta != vertical screen delta, "
            "exception code = %d; "
            "extracted exception code = %d"
            "\n", ecode, extracted_ecode
        );
    default:
        fprintf(
            stderr,
            "Error: unexpected exception, "
            "exception code = %d; "
            "extracted exception code = %d"
            "\n", ecode, extracted_ecode
        );
        break;
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

int main(void) {
    __try {
        kombot_resource_init();
        kombot_user_exit_handler_init();

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    __except(exception_filter(GetExceptionCode())) {
        kombot_resource_freeall();
        ExitProcess(KOMBOT_FAIL);
    }
    return KOMBOT_SUCCESS;
}

/*

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

*/