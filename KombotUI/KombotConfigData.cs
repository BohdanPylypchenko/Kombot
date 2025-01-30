using System.IO;

public struct KombotConfigData
{
    public uint on_off_keycode;
    public uint[] keycodes_no;
    public uint[] keycodes_always;
    public uint[] keycodes_on_target;
    public int mouse_trigger_type;
    public byte target_color_blue;
    public byte target_color_green;
    public byte target_color_red;
    public byte max_target_color_difference_blue;
    public byte max_target_color_difference_green;
    public byte max_target_color_difference_red;
    public int frame_half_wh_px;
    public int screen_width_relation;
    public int screen_height_relation;
    public int mouse_x_ppd;
    public int mouse_y_ppd;
    public int horizontal_fov;
    public double barrier_coefficient;
    public double small_x_coefficient;
    public double small_y_coefficient;

    public void ToFile(string filePath)
    {
        using (FileStream fs = new FileStream(filePath, FileMode.Create, FileAccess.Write))
        using (BinaryWriter writer = new BinaryWriter(fs))
        {
            writer.Write(on_off_keycode);
            WriteKeycodes(writer, keycodes_no);
            WriteKeycodes(writer, keycodes_always);
            WriteKeycodes(writer, keycodes_on_target);
            writer.Write(mouse_trigger_type);
            writer.Write(target_color_blue);
            writer.Write(target_color_green);
            writer.Write(target_color_red);
            writer.Write(max_target_color_difference_blue);
            writer.Write(max_target_color_difference_green);
            writer.Write(max_target_color_difference_red);
            writer.Write(frame_half_wh_px);
            writer.Write(screen_width_relation);
            writer.Write(screen_height_relation);
            writer.Write(mouse_x_ppd);
            writer.Write(mouse_y_ppd);
            writer.Write(horizontal_fov);
            writer.Write(barrier_coefficient);
            writer.Write(small_x_coefficient);
            writer.Write(small_y_coefficient);
        }
    }

    public static KombotConfigData FromFile(string filePath)
    {
        KombotConfigData data = new KombotConfigData();
        using (FileStream fs = new FileStream(filePath, FileMode.Open, FileAccess.Read))
        using (BinaryReader reader = new BinaryReader(fs))
        {
            data.on_off_keycode = reader.ReadUInt32();
            int keycodes_no_count = reader.ReadInt32();
            data.keycodes_no = ReadKeycodes(reader, keycodes_no_count);
            int keycodes_always_count = reader.ReadInt32();
            data.keycodes_always = ReadKeycodes(reader, keycodes_always_count);
            int keycodes_on_target_count = reader.ReadInt32();
            data.keycodes_on_target = ReadKeycodes(reader, keycodes_on_target_count);
            data.mouse_trigger_type = reader.ReadInt32();
            data.target_color_blue = reader.ReadByte();
            data.target_color_green = reader.ReadByte();
            data.target_color_red = reader.ReadByte();
            data.max_target_color_difference_blue = reader.ReadByte();
            data.max_target_color_difference_green = reader.ReadByte();
            data.max_target_color_difference_red = reader.ReadByte();
            data.frame_half_wh_px = reader.ReadInt32();
            data.screen_width_relation = reader.ReadInt32();
            data.screen_height_relation = reader.ReadInt32();
            data.mouse_x_ppd = reader.ReadInt32();
            data.mouse_y_ppd = reader.ReadInt32();
            data.horizontal_fov = reader.ReadInt32();
            data.barrier_coefficient = reader.ReadDouble();
            data.small_x_coefficient = reader.ReadDouble();
            data.small_y_coefficient = reader.ReadDouble();
        }
        return data;
    }

    private static void WriteKeycodes(BinaryWriter writer, uint[] keycodes)
    {
        writer.Write(keycodes.Length);
        foreach (var keycode in keycodes)
            writer.Write(keycode);
    }

    private static uint[] ReadKeycodes(BinaryReader reader, int count)
    {
        uint[] keycodesArray = new uint[count];
        for (int i = 0; i < count; i++)
            keycodesArray[i] = reader.ReadUInt32();
        return keycodesArray;
    }
}
