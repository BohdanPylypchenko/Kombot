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

    UINT screen_resolution_w;
    UINT screen_resolution_h;

    UINT screen_delta;
    LONG aim_frame_wh;

    HBITMAP aim_bitmap_old;
    HBITMAP aim_bitmap;

    BITMAPINFOHEADER aim_bitmap_header;
    DWORD aim_bitmap_size;
    KOMBOT_PTR(BYTE) aim_bitmap_data;

    ULONGLONG frame_count;
} kombot_aim_state;

static kombot_aim_state state;
static char screenshotbuf[128];

static DWORD WINAPI aim_thread_proc(LPVOID parameter) {
    UNREFERENCED_PARAMETER(parameter);

    __try {
        while (state.continue_flag) {
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

            sprintf_s(
                screenshotbuf,
                128 * sizeof(char),
                "screen-%llu.bmp",
                state.frame_count++
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

            UINT target_x = 0;
            UINT target_y = 0;
            UINT border_pixel_count = 0;

            KOMBOT_CONSTREF_RPTR(kombot_bgr_pixel) pixel_arr =
                (KOMBOT_CONSTREF_RPTR(kombot_bgr_pixel))state.aim_bitmap_data;

            for (LONG y = 0; y < state.aim_frame_wh; y++) {
                UINT y_index_offset = y * state.aim_frame_wh;
                for (LONG x = 0; x < state.aim_frame_wh; x++) {
                    DWORD index = y_index_offset + x;
                    KOMBOT_CONSTREF_RPTR(kombot_bgr_pixel) current_pixel = &pixel_arr[index];

                    if (current_pixel->blue == 0 && current_pixel->green == 0 && current_pixel->red == 254) {
                        border_pixel_count++;
                        target_x += x;
                        target_y += y;
                    }
                    else {
                        current_pixel->blue = 0;
                        current_pixel->green = 0;
                        current_pixel->red = 0;
                    }
                }
            }

            if (border_pixel_count != 0) {
                printf("border pixel count = %u\n", border_pixel_count);
                target_x /= border_pixel_count;
                target_y /= border_pixel_count;

                DWORD target_index = target_y * state.aim_frame_wh + target_x;
                KOMBOT_CONSTREF_RPTR(kombot_bgr_pixel) target_pixel = &pixel_arr[target_index];
                target_pixel->blue = 255;
                target_pixel->green = 255;
                target_pixel->red = 255;
            }

            KOMBOT_PTR(FILE) file;
            BITMAPFILEHEADER bfHeader;

            if (fopen_s(&file, screenshotbuf, "wb") != 0) {
                kombot_exception_raise(9999);
            }

            bfHeader.bfType = 0x4D42;
            bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
            bfHeader.bfSize = bfHeader.bfOffBits + state.aim_bitmap_size;
            fwrite(&bfHeader, sizeof(BITMAPFILEHEADER), 1, file);

            fwrite(&state.aim_bitmap_header, sizeof(BITMAPINFOHEADER), 1, file);

            fwrite(state.aim_bitmap_data, state.aim_bitmap_size, 1, file);

            fclose(file);
        }
        return 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        fprintf(
            stderr,
            "Error while executing aim proc; GetLastError = %d\n",
            GetLastError()
        );
        kombot_resource_freeall();
        ExitProcess(KOMBOT_FAIL);
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

    state.frame_count = 0;
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
