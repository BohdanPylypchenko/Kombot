#include "pch.h"

#include "kombot_aim.h"

#include "kombot_exception.h"
#include "kombot_exit.h"
#include "kombot_resource.h"

typedef struct {
    BYTE blue;
    BYTE green;
    BYTE red;
} kombot_bgr_pixel;

typedef struct {
    HANDLE aim_thread;
    BOOL continue_flag;

    HDC h_screen_dc;
    HDC h_memory_dc;

    LONG screen_resolution_w;
    LONG screen_resolution_h;

    LONG screen_delta;
    LONG aim_frame_wh;

    HBITMAP aim_bitmap_old;
    HBITMAP aim_bitmap;

    BITMAPINFOHEADER aim_bitmap_header;
    DWORD aim_bitmap_size;
    KOMBOT_PTR(BYTE) aim_bitmap_data;

    double aim_static_px_rd;

    LONG target_x;
    LONG target_y;

    INPUT aim_input;
} kombot_aim_state;

static kombot_aim_state state;

static KOMBOT_PTR(kombot_bgr_pixel) kombot_aim_capture_frame(void) {
    BitBlt(
        state.h_memory_dc,
        0, 0,
        state.aim_frame_wh,
        state.aim_frame_wh,
        state.h_screen_dc,
        state.screen_resolution_w / 2 - state.screen_delta,
        state.screen_resolution_h / 2 - state.screen_delta,
        SRCCOPY
    );
    
    if (!GetDIBits(
        state.h_memory_dc,
        state.aim_bitmap,
        0, state.aim_frame_wh,
        state.aim_bitmap_data,
        (BITMAPINFO*)&state.aim_bitmap_header,
        DIB_RGB_COLORS
    )) {
        kombot_exception_raise(9999);
    }

    return (KOMBOT_PTR(kombot_bgr_pixel))state.aim_bitmap_data;
}

static BOOL kombot_aim_detect_target(
    KOMBOT_CONST_RPTR(kombot_bgr_pixel) pixel_arr,
    KOMBOT_CONSTREF_RPTR(LONG) target_x,
    KOMBOT_CONSTREF_RPTR(LONG) target_y
) {
    UINT border_pixel_count = 0;
    LONG sum_x = 0;
    LONG sum_y = 0;

    for (LONG y = 0; y < state.aim_frame_wh; y++) {
        LONG y_index_offset = y * state.aim_frame_wh;
        for (LONG x = 0; x < state.aim_frame_wh; x++) {
            DWORD index = y_index_offset + x;
            KOMBOT_CONST_RPTR(kombot_bgr_pixel) current_pixel = &pixel_arr[index];

            if (current_pixel->blue == KOMBOT_AIM_PIXEL_BLUE &&
                current_pixel->green == KOMBOT_AIM_PIXEL_GREEN &&
                current_pixel->red == KOMBOT_AIM_PIXEL_RED
            ) {
                border_pixel_count++;
                sum_x += x;
                sum_y += y;
            }
        }
    }

    if (border_pixel_count == 0) {
        return FALSE;
    }
    else {
        *target_x = sum_x / border_pixel_count;
        *target_y = sum_y / border_pixel_count;
        return TRUE;
    }
}

static void kombot_aim_move_on_target(void) {
    double ddx =
        (double)(state.target_x - state.screen_delta) /
        state.aim_static_px_rd *
        (double)KOMBOT_AIM_MOVE_PX_RD_X;
    double ddy =
        (double)(state.target_y - state.screen_delta) /
        state.aim_static_px_rd *
        (double)KOMBOT_AIM_MOVE_PX_RD_Y;

    state.aim_input.mi.dx = (LONG)trunc(ddx);
    state.aim_input.mi.dy = (LONG)trunc(ddy);
    
    SendInput(1, &state.aim_input, sizeof(INPUT));
}

static DWORD WINAPI aim_thread_proc(LPVOID parameter) {
    UNREFERENCED_PARAMETER(parameter);

    __try {
        while (state.continue_flag) {
            KOMBOT_CONSTVAL_RPTR(kombot_bgr_pixel) pixel_arr = kombot_aim_capture_frame();

            BOOL is_target_detected = kombot_aim_detect_target(
                pixel_arr,
                &state.target_x,
                &state.target_y
            );

            if (!is_target_detected) continue;

            kombot_aim_move_on_target();

            //LONG next_target_x = state.target_x;
            //LONG next_target_y = state.target_y;
            //while (state.continue_flag) {
            //    pixel_arr = kombot_aim_capture_frame();

            //    is_target_detected = kombot_aim_detect_target(
            //        pixel_arr,
            //        &next_target_x,
            //        &next_target_y
            //    );
            //    
            //    if (!is_target_detected ||
            //        fabs(next_target_x - state.target_x) < state.aim_static_px_rd &&
            //        fabs(next_target_y - state.target_y) < state.aim_static_px_rd) {
            //        continue;
            //    }
            //    else {
            //        state.target_x = next_target_x;
            //        state.target_y = next_target_y;

            //        kombot_aim_move_on_target();
            //    }
            //}
        }
        return KOMBOT_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        fprintf(
            stderr,
            "Error while executing aim proc; GetLastError = %d\n",
            GetLastError()
        );
        return KOMBOT_FAIL;
    }
}

void kombot_aim_init(void) {
    state.h_screen_dc = GetDC(NULL);
    state.h_memory_dc = CreateCompatibleDC(state.h_screen_dc);

    state.screen_resolution_w = GetDeviceCaps(state.h_screen_dc, HORZRES);
    state.screen_resolution_h = GetDeviceCaps(state.h_screen_dc, VERTRES);

    state.screen_delta = KOMBOT_AIM_SCREEN_DELTA_PXL;
    state.aim_frame_wh = state.screen_delta * 2;
    state.aim_bitmap = CreateCompatibleBitmap(
        state.h_screen_dc,
        state.aim_frame_wh,
        state.aim_frame_wh
    );
    state.aim_bitmap_old = (HBITMAP)SelectObject(state.h_memory_dc, state.aim_bitmap);

    memset(&state.aim_bitmap_header, 0, sizeof(BITMAPINFOHEADER));

    state.aim_bitmap_header.biSize = sizeof(BITMAPINFOHEADER);
    state.aim_bitmap_header.biWidth = state.aim_frame_wh;
    state.aim_bitmap_header.biHeight = -state.aim_frame_wh;
    state.aim_bitmap_header.biPlanes = 1;
    state.aim_bitmap_header.biBitCount = 24;
    state.aim_bitmap_header.biCompression = BI_RGB;

    state.aim_bitmap_size =
    ((state.aim_frame_wh * state.aim_bitmap_header.biBitCount + 31) / 32) * 4 * state.aim_frame_wh;

    state.aim_bitmap_data = (KOMBOT_PTR(BYTE))HeapAlloc(GetProcessHeap(), 0, state.aim_bitmap_size);
    if (state.aim_bitmap_data == NULL) {
        kombot_exception_raise(KOMBOT_EXCEPTION_AIM_BITMAP_DATA_ALLOC_FAIL);
    }

    double t1 = (double)state.screen_resolution_w / (double)KOMBOT_AIM_HORIZONTAL_FOV;
    double aim_vertical_fow =
    (double)KOMBOT_AIM_HORIZONTAL_FOV / (double)KOMBOT_AIM_SCREEN_RW * (double)KOMBOT_AIM_SCREEN_RH;
    double t2 = (double)state.screen_resolution_h / aim_vertical_fow;
    if (fabs(t1 - t2) > 0.001) {
        kombot_exception_raise(KOMBOT_EXCEPTION_STATIC_PX_RD_RELATION_CHECK_FAIL);
    }
    state.aim_static_px_rd = t1;

    state.aim_input.type = INPUT_MOUSE;
    state.aim_input.mi.dwFlags = MOUSEEVENTF_MOVE;
}

void kombot_aim_start(void) {
    state.continue_flag = TRUE;

    state.aim_thread = CreateThread(
        NULL, 0, aim_thread_proc, NULL, 0, NULL
    );

    if (state.aim_thread == NULL) {
        fprintf(
            stderr,
            "Error: failed to start aim thread, error code = %d\n",
            GetLastError()
        );
        kombot_resource_freeall();
        ExitProcess(KOMBOT_FAIL);
    }
}

void kombot_aim_end(void) {
    state.continue_flag = FALSE;
}

void kombot_aim_free(KOMBOT_PTR(void) pstate) {
    SelectObject(state.h_memory_dc, state.aim_bitmap_old);
    DeleteObject(state.aim_bitmap);
    DeleteDC(state.h_memory_dc);
    ReleaseDC(NULL, state.h_screen_dc);
    HeapFree(GetProcessHeap(), 0, state.aim_bitmap_data);
}
