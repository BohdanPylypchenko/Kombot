export module kombot:frameshot;

import winapi;
using Winapi::Minwindef::Bool;
using Winapi::Minwindef::Word;
using Winapi::Minwindef::Dword;
using Winapi::Display::BgrPixel;
using Winapi::Display::Hdc;
using Winapi::Display::get_dc;
using Winapi::Display::create_compatible_dc;
using Winapi::Display::Hbitmap;
using Winapi::Display::create_compatible_bitmap;
using Winapi::Display::select_object;
using Winapi::Display::delete_object;
using Winapi::Display::delete_dc;
using Winapi::Display::release_dc;
using Winapi::Display::bit_blt;
using Winapi::Display::get_di_bits;
using Winapi::Display::TernaryRasterOperation;
using Winapi::Display::BitmapInfoHeader;
using Winapi::Display::BiCompression;
using Winapi::Display::BitmapInfo;
using Winapi::Display::DibColor;
using Winapi::Display::BitmapFileHeader;

import std;
using std::unique_ptr;
using std::string;
using std::ofstream;
using std::mdspan, std::dextents;

export namespace Kombot::Frameshot
{
    using FrameView = mdspan<BgrPixel, dextents<size_t, 2>>;

    struct HdcScreen
    {
    private:

        Hdc screen;

    public:

        HdcScreen():
            screen(get_dc())
        { }

        HdcScreen(const HdcScreen& other) = default;
        HdcScreen& operator=(const HdcScreen& other) = default;

        HdcScreen(HdcScreen&& other) = default;
        HdcScreen& operator=(HdcScreen&& other) = default;

        inline Hdc get() const
        {
            return screen;
        }

        ~HdcScreen()
        {
            release_dc(screen);
        }
    };

    class FrameShooter
    {
    private:

        static constexpr Word bitmap_rgb_bit_count = 24;
        static constexpr Word bitmap_file_type = 0x4D42;

        int shift_width;
        int shift_height;

        int frame_width;
        int frame_height;

        HdcScreen& screen;
        Hdc memory;

        Hbitmap bitmap;
        Hbitmap old_bitmap;

        BitmapInfoHeader bitmap_info_header;

        Dword bitmap_data_byte_size;
        unique_ptr<char[]> bitmap_data;

    public:

        FrameShooter() = delete;

        FrameShooter(
            HdcScreen& screen,
            int shift_width, int shift_height,
            int frame_width, int frame_height
        ):
            shift_width(shift_width), shift_height(shift_height),
            frame_width(frame_width), frame_height(frame_height),
            screen(screen),
            memory { },
            bitmap { }, old_bitmap { },
            bitmap_info_header { },
            bitmap_data_byte_size
            {
                static_cast<Dword>(((frame_width * bitmap_rgb_bit_count + 31) / 32) * 4 * frame_height)
            },
            bitmap_data { new char[bitmap_data_byte_size] }
        {
            memory = create_compatible_dc(screen.get());

            bitmap = create_compatible_bitmap(screen.get(), frame_width, frame_height);
            old_bitmap = select_object<Hbitmap>(memory, bitmap);

            bitmap_info_header.biSize = sizeof(BitmapInfoHeader);
            bitmap_info_header.biWidth = frame_width;
            bitmap_info_header.biHeight = -frame_height;
            bitmap_info_header.biPlanes = 1;
            bitmap_info_header.biBitCount = bitmap_rgb_bit_count;
            bitmap_info_header.biCompression = static_cast<Dword>(BiCompression::RGB);
        }

        FrameShooter(const FrameShooter& other) = delete;
        FrameShooter& operator=(const FrameShooter& other) = delete;

        FrameShooter(FrameShooter&& other) = default;
        FrameShooter& operator=(FrameShooter&& other) = default;

        inline char* get_buffer() const noexcept
        {
            return bitmap_data.get();
        }

        inline FrameView get_frameview() const noexcept
        {
            return FrameView
            {
                reinterpret_cast<BgrPixel*>(bitmap_data.get()),
                frame_width, frame_height
            };
        }

        inline int get_frame_width() const noexcept
        {
            return frame_width;
        }

        inline int get_frame_height() const noexcept
        {
            return frame_height;
        }

        inline Bool capture_frame_in_bitmap()
        {
            return bit_blt(
                memory,
                0, 0,
                frame_width, frame_height,
                screen.get(),
                shift_width, shift_height,
                TernaryRasterOperation::SrcCopy
            );
        }

        inline Bool copy_bitmap_data_into_buffer()
        {
            return get_di_bits(
                memory, bitmap,
                0, frame_height,
                bitmap_data.get(),
                (BitmapInfo*)&bitmap_info_header,
                DibColor::RGB
            );
        }

        void dump_to_file(const string& file_path) const
        {
            BitmapFileHeader bitmap_file_header { };
            bitmap_file_header.bfType = bitmap_file_type;
            bitmap_file_header.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);
            bitmap_file_header.bfSize = bitmap_file_header.bfOffBits + bitmap_data_byte_size;

            ofstream file(file_path, std::ios::binary);
            file.write(reinterpret_cast<const char*>(&bitmap_file_header), sizeof(BitmapFileHeader));
            file.write(reinterpret_cast<const char*>(&bitmap_info_header), sizeof(BitmapInfoHeader));
            file.write(bitmap_data.get(), bitmap_data_byte_size);
        }

        ~FrameShooter()
        {
            select_object<Hbitmap>(memory, old_bitmap);
            delete_object(bitmap);
            delete_dc(memory);
        }
    };
}