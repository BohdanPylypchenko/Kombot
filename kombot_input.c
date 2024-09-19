#include "pch.h"

#include "kombot_input.h"

#include "kombot_aim.h"
#include "kombot_reftypes.h"
#include "kombot_exception.h"

typedef struct {
    HHOOK keyboard_hook;
    HHOOK mouse_hook;

    BOOL is_key_pressed;
    BOOL is_mouse_pressed;
    BOOL is_aim_active;
} kombot_input_state;

static kombot_input_state state = {
    .keyboard_hook = 0,
    .mouse_hook = 0,
    .is_key_pressed = FALSE,
    .is_mouse_pressed = FALSE,
    .is_aim_active = FALSE
};

static void check_aim_activation(void) {
    if ((state.is_key_pressed || state.is_mouse_pressed) && !state.is_aim_active) {
        state.is_aim_active = TRUE;
        kombot_aim_start();
        printf("is aim activated = %d\n", state.is_aim_active);
        return;
    }

    if (!state.is_key_pressed && !state.is_mouse_pressed && state.is_aim_active) {
        state.is_aim_active = FALSE;
        kombot_aim_end();
        printf("is aim deactivated = %d\n", state.is_aim_active);
        return;
    }
}

static LRESULT CALLBACK kombot_keyboard_proc(
    int code,
    WPARAM wparam, LPARAM lparam
) {
    if (code == HC_ACTION && WM_KEYFIRST <= wparam && wparam <= WM_KEYLAST) {
        KOMBOT_CONST_RPTR(KBDLLHOOKSTRUCT) keyboard =
            (KOMBOT_PTR(KBDLLHOOKSTRUCT))lparam;
        if (keyboard->vkCode == KOMBOT_ACTIVATION_KEY) {
            if (wparam == WM_KEYDOWN) {
                state.is_key_pressed = TRUE;
                printf("kombot activation key pressed: '%c'\n", KOMBOT_ACTIVATION_KEY);
                check_aim_activation();
            }
            else if (wparam == WM_KEYUP) {
                state.is_key_pressed = FALSE;
                printf("kombot activation key released: '%c'\n", KOMBOT_ACTIVATION_KEY);
                check_aim_activation();
            }
        }
    }

    return CallNextHookEx(NULL, code, wparam, lparam);
}

static LRESULT CALLBACK kombot_mouse_proc(
    int code,
    WPARAM wparam, LPARAM lparam
) {
    if (code == HC_ACTION) {
        if (wparam == KOMBOT_ACTIVATION_MOUSE) {
            state.is_mouse_pressed = TRUE;
            printf("kombot mouse left click pressed\n");
            check_aim_activation();
        }
        else if (wparam == KOMBOT_DEACTIVATION_MOUSE) {
            state.is_mouse_pressed = FALSE;
            printf("kombot mouse left click released\n");
            check_aim_activation();
        }
    }

    return CallNextHookEx(NULL, code, wparam, lparam);
}

void kombot_input_init(void) {
    state.keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, kombot_keyboard_proc, NULL, 0);
    state.mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, kombot_mouse_proc, NULL, 0);
    //state.mouse_hook = 1;

    if (!state.keyboard_hook || !state.mouse_hook) {
        kombot_exception_raise(KOMBOT_EXCEPTION_INPUT_INIT);
    }
}

void kombot_input_free(KOMBOT_PTR(void) pstate) {
    UNREFERENCED_PARAMETER(pstate);

    UnhookWindowsHookEx(state.keyboard_hook);
    UnhookWindowsHookEx(state.mouse_hook);
}