#include "pch.h"

#include "kombot_exit.h"

#include "Windows.h"
#include "kombot_resource.h"
#include "kombot_exception.h"

static BOOL WINAPI kombot_exit_handler(DWORD ctrl_type) {
    if (ctrl_type == CTRL_C_EVENT || ctrl_type == CTRL_CLOSE_EVENT) {
        kombot_resource_freeall();
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void kombot_user_exit_handler_init(void) {
    if (!SetConsoleCtrlHandler(kombot_exit_handler, TRUE)) {
        kombot_exception_raise(GetLastError());
    }
}