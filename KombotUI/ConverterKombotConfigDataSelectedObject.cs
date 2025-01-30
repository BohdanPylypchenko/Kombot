using System.Linq;

namespace KombotUI
{
    internal static class ConverterKombotConfigDataSelectedObject
    {
        public static KombotConfigSelectedObject ToSelectedObject(KombotConfigData kcd, string path)
        {
            KombotConfigSelectedObject result = new KombotConfigSelectedObject();

            result.SetPath(path);

            result.OnOffKeycode = (char)kcd.on_off_keycode;

            result.KeycodesNo = kcd.keycodes_no.Select(k => (char)k).ToArray();
            result.KeycodesAlways = kcd.keycodes_always.Select(k => (char)k).ToArray();
            result.KeycodesOnTarget = kcd.keycodes_on_target.Select(k => (char)k).ToArray();

            result.MouseTrigger = (MouseTriggerButton)kcd.mouse_trigger_type;

            result.TargetColor = new BgrColor(
                kcd.target_color_blue,
                kcd.target_color_green,
                kcd.target_color_red
            );
            result.MaxTargetColorDifference = new BgrColor(
                kcd.max_target_color_difference_blue,
                kcd.max_target_color_difference_green,
                kcd.max_target_color_difference_red
            );

            result.FrameHalfWidthHeightPx = kcd.frame_half_wh_px;
            result.ScreenWidthRelation = kcd.screen_width_relation;
            result.ScreenHeightRelation = kcd.screen_height_relation;
            result.MouseXPPD = kcd.mouse_x_ppd;
            result.MouseYPPD = kcd.mouse_y_ppd;
            result.HorizontalFov = kcd.horizontal_fov;
            result.BarrierCoefficient = kcd.barrier_coefficient;
            result.SmallXCoefficient = kcd.small_x_coefficient;
            result.SmallYCoefficient = kcd.small_y_coefficient;

            return result;
        }

        public static KombotConfigData ToConfigData(KombotConfigSelectedObject kcso)
        {
            KombotConfigData result = new KombotConfigData();

            result.on_off_keycode = kcso.OnOffKeycode;

            result.keycodes_no = kcso.KeycodesNo.Select(k => (uint)k).ToArray();
            result.keycodes_always = kcso.KeycodesAlways.Select(k => (uint)k).ToArray();
            result.keycodes_on_target = kcso.KeycodesOnTarget.Select(k => (uint)k).ToArray();

            result.mouse_trigger_type = (int)kcso.MouseTrigger;

            result.target_color_blue = (byte)kcso.TargetColor.Blue;
            result.target_color_green = (byte)kcso.TargetColor.Green;
            result.target_color_red = (byte)kcso.TargetColor.Red;

            result.max_target_color_difference_blue = (byte)kcso.MaxTargetColorDifference.Blue;
            result.max_target_color_difference_green = (byte)kcso.MaxTargetColorDifference.Green;
            result.max_target_color_difference_red = (byte)kcso.MaxTargetColorDifference.Red;

            result.frame_half_wh_px = kcso.FrameHalfWidthHeightPx;
            result.screen_width_relation = kcso.ScreenWidthRelation;
            result.screen_height_relation = kcso.ScreenHeightRelation;
            result.mouse_x_ppd = kcso.MouseXPPD;
            result.mouse_y_ppd = kcso.MouseYPPD;
            result.horizontal_fov = kcso.HorizontalFov;
            result.barrier_coefficient = kcso.BarrierCoefficient;
            result.small_x_coefficient = kcso.SmallXCoefficient;
            result.small_y_coefficient = kcso.SmallYCoefficient;

            return result;
        }
    }
}
