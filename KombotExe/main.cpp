import winapi;
using Winapi::Minwindef::Dword;
using Winapi::Minwindef::Byte;
using Winapi::Minwindef::Bool;
using Winapi::Minwindef::True;
using Winapi::Minwindef::False;
using Winapi::Consoleapi::ControlEvent;
using Winapi::Consoleapi::set_console_control_handler;

import kombot;
using Kombot::Konfig::Config;
using Kombot::Konfig::KeycodeSet;
using Kombot::Konfig::MouseTriggerType;

import std;
using std::string;
using std::ifstream;
using std::println, std::print;
using std::span;
using std::cin;

struct KombotConfigData
{
public:

    Dword on_off_keycode;
    int keycodes_no_count; Dword* keycodes_no;
    int keycodes_always_count; Dword* keycodes_always;
    int keycodes_on_target_count; Dword* keycodes_on_target;
    int mouse_trigger_type;
    Byte target_color_blue; Byte target_color_green; Byte target_color_red;
    Byte max_target_color_difference_blue; Byte max_target_color_difference_green; Byte max_target_color_differencce_red;
    int frame_half_wh_px;
    int screen_width_relation;
    int screen_height_relation;
    int mouse_x_ppd;
    int mouse_y_ppd;
    int horizontal_fov;
    double barrier_coefficient;
    double small_x_coefficient;
    double small_y_coefficient;

private:

    KombotConfigData() = default;

public:

    KombotConfigData(const KombotConfigData& other) = default;
    KombotConfigData& operator=(const KombotConfigData& other) = default;

    KombotConfigData(KombotConfigData&& other) = default;
    KombotConfigData& operator=(KombotConfigData&& other) = default;

    static KombotConfigData from_file(ifstream& file)
    {
        KombotConfigData result { };

        file.read(reinterpret_cast<char*>(&result.on_off_keycode), sizeof(Dword));

        file.read(reinterpret_cast<char*>(&result.keycodes_no_count), sizeof(int));
        result.keycodes_no = allocate_read_keycodes(file, result.keycodes_no_count);

        file.read(reinterpret_cast<char*>(&result.keycodes_always_count), sizeof(int));
        result.keycodes_always = allocate_read_keycodes(file, result.keycodes_always_count);

        file.read(reinterpret_cast<char*>(&result.keycodes_on_target_count), sizeof(int));
        result.keycodes_on_target = allocate_read_keycodes(file, result.keycodes_on_target_count);

        file.read(reinterpret_cast<char*>(&result.mouse_trigger_type), sizeof(int));

        file.read(reinterpret_cast<char*>(&result.target_color_blue), sizeof(Byte));
        file.read(reinterpret_cast<char*>(&result.target_color_green), sizeof(Byte));
        file.read(reinterpret_cast<char*>(&result.target_color_red), sizeof(Byte));

        file.read(reinterpret_cast<char*>(&result.max_target_color_difference_blue), sizeof(Byte));
        file.read(reinterpret_cast<char*>(&result.max_target_color_difference_green), sizeof(Byte));
        file.read(reinterpret_cast<char*>(&result.max_target_color_differencce_red), sizeof(Byte));

        file.read(reinterpret_cast<char*>(&result.frame_half_wh_px), sizeof(int));
        file.read(reinterpret_cast<char*>(&result.screen_width_relation), sizeof(int));
        file.read(reinterpret_cast<char*>(&result.screen_height_relation), sizeof(int));
        file.read(reinterpret_cast<char*>(&result.mouse_x_ppd), sizeof(int));
        file.read(reinterpret_cast<char*>(&result.mouse_y_ppd), sizeof(int));
        file.read(reinterpret_cast<char*>(&result.horizontal_fov), sizeof(int));
        file.read(reinterpret_cast<char*>(&result.barrier_coefficient), sizeof(double));
        file.read(reinterpret_cast<char*>(&result.small_x_coefficient), sizeof(double));
        file.read(reinterpret_cast<char*>(&result.small_y_coefficient), sizeof(double));

        return result;
    }

    static KeycodeSet to_keycode_set(int keycodes_count, DWORD* keycodes_arr)
    {
        span<DWORD> keycodes { keycodes_arr, static_cast<size_t>(keycodes_count) };
        return KeycodeSet(keycodes.begin(), keycodes.end());
    }

    ~KombotConfigData()
    {
        delete[] keycodes_no;
        delete[] keycodes_always;
        delete[] keycodes_on_target;
    }

private:

    static Dword* allocate_read_keycodes(ifstream& file, int count)
    {
        Dword* result = new Dword[count];
        file.read(reinterpret_cast<char*>(result), count * sizeof(Dword));
        return result;
    }
};

static Bool exit_handler(Dword control_event)
{
    if (
        control_event == static_cast<Dword>(ControlEvent::C) ||
        control_event == static_cast<Dword>(ControlEvent::Close) ||
        control_event == static_cast<Dword>(ControlEvent::Logoff) ||
        control_event == static_cast<Dword>(ControlEvent::Shutdown)
    ) {
        Kombot::cleanup();
    }
    return False;
}

static const char run_answer = '+';

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        println("KombotExe [path-to-config_file : str]");
        return 1;
    }

    if (set_console_control_handler(exit_handler, True) != 0)
    {
        println("Failed to register playground destructor using atexit");
        return 1;
    }

    string path(argv[1]);
    ifstream config_file(path, std::ios::binary);
    if (!config_file)
    {
        println("Error: failed to open config_file {}", path);
        return 1;
    }

    KombotConfigData config_data { KombotConfigData::from_file(config_file) };
    config_file.close();

    Config config
    {
        .on_off_keycode = config_data.on_off_keycode,
        .aim_config =
        {
            .target_color =
            {
                .blue = config_data.target_color_blue,
                .green = config_data.target_color_green,
                .red = config_data.target_color_red
            },
            .max_target_color_difference =
            {
                .blue = config_data.max_target_color_difference_blue,
                .green = config_data.max_target_color_difference_green,
                .red = config_data.max_target_color_differencce_red
            },
            .frame_half_wh_px = config_data.frame_half_wh_px,
            .screen_width_relation = config_data.screen_width_relation,
            .screen_height_relation = config_data.screen_height_relation,
            .mouse_x_ppd = config_data.mouse_x_ppd,
            .mouse_y_ppd = config_data.mouse_y_ppd,
            .horizontal_fov = config_data.horizontal_fov,
            .barrier_coefficient = config_data.barrier_coefficient,
            .small_x_coefficient = config_data.small_x_coefficient,
            .small_y_coefficient = config_data.small_y_coefficient
        },
        .keycodes_no = KombotConfigData::to_keycode_set(
            config_data.keycodes_no_count,
            config_data.keycodes_no
        ),
        .keycodes_always = KombotConfigData::to_keycode_set(
            config_data.keycodes_always_count,
            config_data.keycodes_always
        ),
        .keycodes_on_target = KombotConfigData::to_keycode_set(
            config_data.keycodes_on_target_count,
            config_data.keycodes_on_target
        ),
        .mouse_trigger_type = static_cast<MouseTriggerType>(config_data.mouse_trigger_type)
    };

    print("\n\n===\n\n");
    println("{}", config.to_string());
    print("\n\n===\n\n");

    println(
        "Please review kombot parameters.\nTo Run kombot, enter '{}'.\nTo exit, enter any other char:",
        run_answer
    );
    char answer { };
    cin >> answer;
    if (answer != run_answer)
    {
        return 0;
    }

    Kombot::run(config);

    return 0;
}