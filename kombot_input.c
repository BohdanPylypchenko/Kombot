#include "pch.h"

#include "kombot_input.h"

#include "kombot_reftypes.h"
#include "kombot_exception.h"

typedef struct {
    HHOOK keyboard_hook;
    HHOOK mouse_hook;
} kombot_input_state;

static kombot_input_state state;

static LRESULT CALLBACK kombot_keyboard_proc(
    int code,
    WPARAM wparam, LPARAM lparam
) {
    if (code == HC_ACTION && wparam == WM_KEYDOWN) {
        KOMBOT_CONST_RPTR(KBDLLHOOKSTRUCT) keyboard =
            (KOMBOT_PTR(KBDLLHOOKSTRUCT))lparam;
        if (keyboard->vkCode == KOMBOT_ACTIVATION_KEY) {
            // TO-DO: call something
            printf("kombot activation key pressed: '%c'\n", KOMBOT_ACTIVATION_KEY);
        }
    }

    return CallNextHookEx(NULL, code, wparam, lparam);
}

static LRESULT CALLBACK kombot_mouse_proc(
    int code,
    WPARAM wparam, LPARAM lparam
) {
    if (code == HC_ACTION && wparam == KOMBOT_ACTIVATION_MOUSE) {
        // TO-DO: call something
        printf("kombot mouse left click pressed\n");
    }

    return CallNextHookEx(NULL, code, wparam, lparam);
}

void kombot_input_state_init(void) {
    state.keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, kombot_keyboard_proc, NULL, 0);
    state.mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, kombot_mouse_proc, NULL, 0);

    if (!state.keyboard_hook || !state.mouse_hook) {
        kombot_raise_exception(KOMBOT_EXCEPTION_INPUT_INIT);
    }
}

void kombot_input_state_free(KOMBOT_PTR(void) pstate) {
    UNREFERENCED_PARAMETER(pstate);

    UnhookWindowsHookEx(state.keyboard_hook);
    UnhookWindowsHookEx(state.mouse_hook);
}