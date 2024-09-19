#include "pch.h"

#include "kombot_aim.h"

#include "kombot_exception.h"
#include "kombot_exit.h"
#include "kombot_resource.h"

typedef struct {
    HANDLE aim_thread;
    BOOL continue_flag;

    HDC h_screen_dc;
    HDC h_memory_dc;

    int screen_resolution_w;
    int screen_resolution_h;

    int screen_delta;
    int aim_frame_wh;

    HBITMAP aim_bitmap_old;
    HBITMAP aim_bitmap;

    ULONGLONG frame_count;
} kombot_aim_state;

static kombot_aim_state state;
static char screenshotbuf[128];

// Function to save the bitmap to a file
static void SaveBitmapToFile(HBITMAP hBitmap, HDC hDC, int width, int height, const char * filePath) {
    BITMAPFILEHEADER bfHeader;
    BITMAPINFOHEADER biHeader;
    BITMAPINFO bInfo;
    FILE * file;
    char * bmpData;
    int size;

    // Initialize headers
    memset(&bfHeader, 0, sizeof(BITMAPFILEHEADER));
    memset(&biHeader, 0, sizeof(BITMAPINFOHEADER));
    memset(&bInfo, 0, sizeof(BITMAPINFO));

    biHeader.biSize = sizeof(BITMAPINFOHEADER);
    biHeader.biWidth = width;
    biHeader.biHeight = -height; // negative to prevent flipping
    biHeader.biPlanes = 1;
    biHeader.biBitCount = 24; // 24 bits for RGB
    biHeader.biCompression = BI_RGB;

    size = ((width * biHeader.biBitCount + 31) / 32) * 4 * height; // Calculate bitmap size

    bmpData = (char *)HeapAlloc(GetProcessHeap(), 0, size);

    // Get the bitmap data from the HBITMAP
    if (!GetDIBits(hDC, hBitmap, 0, height, bmpData, (BITMAPINFO *)&biHeader, DIB_RGB_COLORS)) {
        HeapFree(GetProcessHeap(), 0, bmpData);
        return;
    }

    // Create a file to save the bitmap
    if (fopen_s(&file, filePath, "wb") != 0) {
        fprintf(stderr, "Error: can't create result file\n");
        HeapFree(GetProcessHeap(), 0, bmpData);
        return;
    }

    // Write the file header
    bfHeader.bfType = 0x4D42; // 'BM'
    bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfHeader.bfSize = bfHeader.bfOffBits + size;
    fwrite(&bfHeader, sizeof(BITMAPFILEHEADER), 1, file);

    // Write the info header
    fwrite(&biHeader, sizeof(BITMAPINFOHEADER), 1, file);

    // Write the bitmap data
    fwrite(bmpData, size, 1, file);

    // Close the file and free memory
    fclose(file);
    HeapFree(GetProcessHeap(), 0, bmpData);
}

static DWORD WINAPI aim_thread_proc(LPVOID parameter) {
    UNREFERENCED_PARAMETER(parameter);

    __try {
        while (state.continue_flag) {
            //printf("aiming...\n");
            //Sleep(100);

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

            SaveBitmapToFile(
                state.aim_bitmap,
                state.h_memory_dc,
                state.aim_frame_wh,
                state.aim_frame_wh,
                screenshotbuf
            );
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

    int t1 = state.screen_resolution_w / KOMBOT_SCREEN_HPROP0RTION;
    int t2 = state.screen_resolution_h / KOMBOT_SCREEN_VPROP0RTION;
    if (t1 != t2) {
        kombot_exception_raise(KOMBOT_EXCEPTION_AIM_DELTA_FAIL);
    }
    state.screen_delta = t1;

    state.aim_frame_wh = state.screen_delta * 2;
    state.aim_bitmap = CreateCompatibleBitmap(
        state.h_screen_dc,
        state.aim_frame_wh,
        state.aim_frame_wh
    );
    state.aim_bitmap_old = (HBITMAP)SelectObject(state.h_memory_dc, state.aim_bitmap);

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
}
