#include "pch.h"

#include "kombot_exception.h"

void kombot_exception_raise(kombot_exception_code kec) {
    DWORD pkec = 0;
    KOMBOT_EXCEPTION_EXTRACT_ERROR_CODE(pkec, kec);

    pkec |= (1 << 28);
    pkec |= (1 << 29);
    pkec |= (1 << 30);

    RaiseException(
        pkec, 0, 0, NULL
    );
}