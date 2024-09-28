#include "pch.h"

#include "kombot_input.h"

#include "kombot_aim.h"
#include "kombot_reftypes.h"
#include "kombot_exception.h"

typedef struct {
    HHOOK keyboard_hook;
    BOOL is_aim_active;
} kombot_input_state;

static kombot_input_state state = {
    .keyboard_hook = 0,
    .is_aim_active = FALSE
};

static LRESULT CALLBACK kombot_keyboard_proc(
    int code,
    WPARAM wparam, LPARAM lparam
) {
    if (code == HC_ACTION && WM_KEYFIRST <= wparam && wparam <= WM_KEYLAST) {
        KOMBOT_CONST_RPTR(KBDLLHOOKSTRUCT) keyboard = (KOMBOT_PTR(KBDLLHOOKSTRUCT))lparam;
        if (keyboard->vkCode == KOMBOT_ACTIVATION_KEY_REGULAR || keyboard->vkCode == KOMBOT_ACTIVATION_KEY_SYSTEM) {
            if (wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN) {
                if (!state.is_aim_active) {
                    state.is_aim_active = TRUE;
                    kombot_aim_start();
                    printf("kombot aim start, key: '%d'\n", keyboard->vkCode);
                }
            }
            else if (wparam == WM_KEYUP || wparam == WM_SYSKEYUP) {
                state.is_aim_active = FALSE;
                kombot_aim_end();
                printf("kombot aim end, key: '%d'\n", keyboard->vkCode);
            }
        }
    }

    return CallNextHookEx(NULL, code, wparam, lparam);
}

void kombot_input_init(void) {
    state.keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, kombot_keyboard_proc, NULL, 0);

    if (!state.keyboard_hook) {
        kombot_exception_raise(KOMBOT_EXCEPTION_INPUT_INIT);
    }
}

void kombot_input_free(KOMBOT_PTR(void) pstate) {
    UNREFERENCED_PARAMETER(pstate);

    UnhookWindowsHookEx(state.keyboard_hook);
}