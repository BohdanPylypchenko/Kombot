#pragma once

#include "Windows.h"

typedef enum {
    KOMBOT_EXCEPTION_EXIT_HANDLER_INIT = 1,
    KOMBOT_EXCEPTION_INPUT_INIT
} kombot_exception_code;

#define KOMBOT_EXCEPTION_EXTRACT_ERROR_CODE(dst, src) \
{                                                     \
    dst = (DWORD)0;                                   \
    dst = (DWORD)src & 0x7FFFFFF;                     \
}

void kombot_raise_exception(kombot_exception_code kec);
