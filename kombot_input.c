#include "pch.h"

#include "kombot_input.h"

#include "kombot_error.h"
#include "kombot_reftypes.h"

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
    if (code == HC_ACTION && wparam == WM_LBUTTONDOWN) {
        // TO-DO: call something
        printf("kombot mouse left click pressed\n");
    }

    return CallNextHookEx(NULL, code, wparam, lparam);
}

KOMBOT_PTR(kombot_input_state) kombot_input_state_init(
    KOMBOT_CONSTREF_RPTR(kombot_input_state) state
) {
    state->keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, kombot_keyboard_proc, NULL, 0);
    state->mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, kombot_mouse_proc, NULL, 0);

    if (state->keyboard_hook && state->mouse_hook) {
        return state;
    } else {
        kombot_error_set_global(
            KOMBOT_ERROR_INPUT_INIT,
            GetLastError()
        );
        return NULL;
    }
}

void kombot_input_state_free(
    KOMBOT_CONST_RPTR(kombot_input_state) state
) {
    UnhookWindowsHookEx(state->keyboard_hook);
    UnhookWindowsHookEx(state->mouse_hook);
}