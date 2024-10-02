#include "pch.h"

#define MOUSE_NULLIFY_COUNTER_KEY '5'
#define MOUSE_END_CAPTURE_KEY '6'
#define MOUSE_MOVE_X_KEY '7'
#define MOUSE_MOVE_Y_KEY '8'
#define MOUSE_MOVE_X_LARGE_KEY '9'
#define MOUSE_MOVE_Y_LARGE_KEY '0'

static const double x_rd_count = 360;
static const double y_rd_count = 90;

static const LONG x_rd_d = 10;
static const LONG y_rd_d = 10;

static const LONG x_rd_dl = 32730;
static const LONG y_rd_dl = 16170;

typedef struct {
    HHOOK keyboard_hook;
    INPUT input;
    ULONG x_px_counter;
    ULONG y_px_counter;
    ULONG mouse_move_count;
} ppd_state;

static ppd_state state = {
    .keyboard_hook = 0,
    .input = 0,
    .x_px_counter = 0,
    .y_px_counter = 0,
    .mouse_move_count = 0
};

static BOOL WINAPI ppd_exit_handler(DWORD ctrl_type) {
    if (ctrl_type == CTRL_C_EVENT || ctrl_type == CTRL_CLOSE_EVENT)
        UnhookWindowsHookEx(state.keyboard_hook);
    return FALSE;
}

static void ppd_nullify_counter(void) {
    state.x_px_counter = 0;
    state.y_px_counter = 0;
    state.mouse_move_count = 0;
}

static void ppd_move_mouse_x(void) {
    state.input.mi.dx = x_rd_d;
    state.input.mi.dy = 0;
    SendInput(1, &state.input, sizeof(INPUT));
    state.x_px_counter += state.input.mi.dx;
    state.mouse_move_count++;
}

static void ppd_move_mouse_y(void) {
    state.input.mi.dx = 0;
    state.input.mi.dy = y_rd_d;
    SendInput(1, &state.input, sizeof(INPUT));
    state.y_px_counter += state.input.mi.dy;
    state.mouse_move_count++;
}

static void ppd_move_mouse_x_large(void) {
    state.input.mi.dx = x_rd_dl;
    state.input.mi.dy = 0;
    SendInput(1, &state.input, sizeof(INPUT));
}

static void ppd_move_mouse_y_large(void) {
    state.input.mi.dx = 0;
    state.input.mi.dy = y_rd_dl;
    SendInput(1, &state.input, sizeof(INPUT));
}

static void ppd_state_capture_mouse_move_count(void) {
    double rx = (double)state.x_px_counter / x_rd_count;
    double ry = (double)state.y_px_counter / y_rd_count;
    printf(
        "rx = %d / %lf = %lf\n"
        "ry = %d / %lf = %lf\n",
        state.x_px_counter, x_rd_count, rx,
        state.y_px_counter, y_rd_count, ry
    );
}

static LRESULT CALLBACK ppd_keyboard_proc(
    int code,
    WPARAM wparam, LPARAM lparam
) {
    if (code == HC_ACTION && wparam == WM_KEYDOWN) {
        const KBDLLHOOKSTRUCT * const keyboard = (KBDLLHOOKSTRUCT*)lparam;

        switch (keyboard->vkCode) {
        case MOUSE_NULLIFY_COUNTER_KEY:
            printf("px counter nullified: %c\n", MOUSE_NULLIFY_COUNTER_KEY);
            ppd_nullify_counter();
            break;
        case MOUSE_MOVE_X_KEY:
            ppd_move_mouse_x();
            break;
        case MOUSE_MOVE_Y_KEY:
            ppd_move_mouse_y();
            break;
        case MOUSE_MOVE_X_LARGE_KEY:
            ppd_move_mouse_x_large();
            break;
        case MOUSE_MOVE_Y_LARGE_KEY:
            ppd_move_mouse_y_large();
            break;
        case MOUSE_END_CAPTURE_KEY:
            printf("px move count captured: %c\n", MOUSE_END_CAPTURE_KEY);
            ppd_state_capture_mouse_move_count();
            break;
        default:
            break;
        }
    }

    return CallNextHookEx(NULL, code, wparam, lparam);
}

int main(void) {
    if (!SetConsoleCtrlHandler(ppd_exit_handler, TRUE)) {
        fprintf(stderr, "Error: failed to set exit handle, GetLastError = %d\n", GetLastError());
        return 1;
    }

    state.keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, ppd_keyboard_proc, NULL, 0);
    if (!state.keyboard_hook) {
        fprintf(stderr, "Error: failed to initialize keyboard hook, GetLastError = %d\n", GetLastError());
        return 1;
    }

    state.input.type = INPUT_MOUSE;
    state.input.mi.dwFlags = MOUSEEVENTF_MOVE;

    printf("ppd calculator started\n");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}