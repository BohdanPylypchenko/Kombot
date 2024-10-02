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
            "GetLastError = %d; "
            "extracted exception code = %d"
            "\n", GetLastError(), extracted_ecode
        );
        break;
    case KOMBOT_EXCEPTION_INPUT_INIT:
        fprintf(
            stderr,
            "Error: failed to initialize keyboard/mouse hooks, "
            "GetLastError = %d; "
            "extracted exception code = %d"
            "\n", GetLastError(), extracted_ecode
        );
        break;
    case KOMBOT_EXCEPTION_AIM_BITMAP_DATA_ALLOC_FAIL:
        fprintf(
            stderr,
            "Error: failed to allocate memory for aim frame, "
            "GetLastError = %d; "
            "extracted exception code = %d"
            "\n", GetLastError(), extracted_ecode
        );
        break;
    case KOMBOT_EXCEPTION_STATIC_PX_RD_RELATION_CHECK_FAIL:
        fprintf(
            stderr,
            "Error: failed static pixel per rotation degree relation check\n"
        );
        break;
    default:
        fprintf(
            stderr,
            "Error: unexpected exception, "
            "GetLastError = %d; "
            "extracted exception code = %d"
            "\n", GetLastError(), extracted_ecode
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
