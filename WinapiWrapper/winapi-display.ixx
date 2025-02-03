module;

#include "pch.h"

export module winapi.display;

export import winapi.minwindef;
using Winapi::Minwindef::Byte;
using Winapi::Minwindef::Bool;
using Winapi::Minwindef::Dword;
using Winapi::Minwindef::UInt;

export namespace Winapi::Display
{
    using HGDIObject = HGDIOBJ;
    using Hdc = HDC;
    using Hwnd = HWND;
    using Hbitmap = HBITMAP;
    using BitmapInfo = BITMAPINFO;
    using BitmapInfoHeader = BITMAPINFOHEADER;
    using BitmapFileHeader = BITMAPFILEHEADER;

    enum class DeviceCap
    {
        HorizontalResolution = HORZRES,
        VecticalResolution = VERTRES
    };

    enum class BiCompression
    {
        RGB = BI_RGB,
        RLE8 = BI_RLE8,
        RLE4 = BI_RLE4,
        BITFIELDS = BI_BITFIELDS,
        JPEG = BI_JPEG,
        PNG = BI_PNG
    };

    enum class TernaryRasterOperation
    {
        SrcCopy = SRCCOPY,
        SrcPaint = SRCPAINT,
        SrcAnd = SRCAND,
        SrcInvert = SRCINVERT,
        SrcErase = SRCERASE,
        NotSrcCopy = NOTSRCCOPY,
        NotSrcErase = NOTSRCERASE,
        MergeCopy = MERGECOPY,
        MergePaint = MERGEPAINT,
        PatCopy = PATCOPY,
        PatPaint = PATPAINT,
        PatInvert = PATINVERT,
        DstInvert = DSTINVERT,
        Black = BLACKNESS,
        White = WHITENESS,
    };

    enum class DibColor
    {
        RGB = DIB_RGB_COLORS,
        PAL = DIB_PAL_COLORS
    };

    struct BgrPixel
    {
        Byte blue;
        Byte green;
        Byte red;
    };

    inline Hdc get_dc(Hwnd hwnd)
    {
        return GetDC(hwnd);
    }

    inline Hdc get_dc()
    {
        return GetDC(NULL);
    }

    inline Hdc create_compatible_dc(Hdc hdc)
    {
        return CreateCompatibleDC(hdc);
    }

    inline int get_device_caps(Hdc hdc, DeviceCap device_cap)
    {
        return GetDeviceCaps(hdc, static_cast<int>(device_cap));
    }

    inline Hbitmap create_compatible_bitmap(Hdc hdc, int cx, int cy)
    {
        return CreateCompatibleBitmap(hdc, cx, cy);
    }

    template<typename T>
    inline T select_object(Hdc hdc, HGDIObject gdi_obj)
    {
        return reinterpret_cast<T>(SelectObject(hdc, gdi_obj));
    }

    inline Bool delete_object(HGDIObject gdi_obj)
    {
        return DeleteObject(gdi_obj);
    }

    inline Bool delete_dc(Hdc hdc)
    {
        return DeleteDC(hdc);
    }

    inline int release_dc(Hwnd hwnd, Hdc hdc)
    {
        return ReleaseDC(hwnd, hdc);
    }

    inline int release_dc(Hdc hdc)
    {
        return ReleaseDC(NULL, hdc);
    }

    inline Bool bit_blt(
        Hdc hdc, int x, int y, int cx, int cy,
        Hdc hdc_src, int x1, int y1, TernaryRasterOperation rop
    ) {
        return BitBlt(hdc, x, y, cx, cy, hdc_src, x1, y1, static_cast<Dword>(rop));
    }

    inline int get_di_bits(
        Hdc hdc, Hbitmap bitmap,
        UInt start, UInt line_count,
        void* bits, BitmapInfo* bitmap_info,
        DibColor color
    ) {
        return GetDIBits(
            hdc, bitmap,
            start, line_count,
            bits,
            bitmap_info, static_cast<UInt>(color)
        );
    }
}
