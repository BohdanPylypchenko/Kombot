using System.ComponentModel;

namespace KombotUI
{
    internal class KombotConfigSelectedObject
    {
        private string path;

        [Description("Path to currently opened config")]
        [Category("Misc")]
        public string Path => path;

        public void SetPath(string path)
        {
            this.path = path;
        }

        [Description("Keycode to enable/disable kombot when running")]
        [Category("Keycodes")]
        public char OnOffKeycode { get; set; }

        [Description("Keycodes (weapons) to shoot with no auto-aim (example: rocket)")]
        [Category("Keycodes")]
        public char[] KeycodesNo { get; set; }

        [Description("Keycodes (weapons) to always shoot with auto-aim (example: machinegun)")]
        [Category("Keycodes")]
        public char[] KeycodesAlways { get; set; }

        [Description("Keycodes (weapons) to shoot with auto-aim only when on target (example: railgun)")]
        [Category("Keycodes")]
        public char[] KeycodesOnTarget { get; set; }

        [Description("Mouse button to trigger auto-aim")]
        [Category("Triggers")]
        public MouseTriggerButton MouseTrigger { get; set; }

        [Description("Base color to aim on (as RGb)")]
        [Category("Target")]
        public BgrColor TargetColor { get; set; }

        [Description("Color difference to aim on (as RGb)")]
        [Category("Target")]
        public BgrColor MaxTargetColorDifference { get; set; }

        [Description("Width/Height of screen area to scan with kombot (in pixels)")]
        [Category("Game parameters")]
        public int FrameHalfWidthHeightPx { get; set; }

        [Description("Screen width relation. Example (16):9")]
        [Category("Game parameters")]
        public int ScreenWidthRelation { get; set; }

        [Description("Screen height relation. Example 16:(9)")]
        [Category("Game parameters")]
        public int ScreenHeightRelation { get; set; }

        [Description("Use PixelsPerDegreeCalculator to calculate this parametr")]
        [Category("Game parameters")]
        public int MouseXPPD { get; set; }

        [Description("Use PixelsPerDegreeCalculator to calculate this parametr")]
        [Category("Game parameters")]
        public int MouseYPPD { get; set; }

        [Description("Game parameter: how 'wide' is view")]
        [Category("Game parameters")]
        public int HorizontalFov { get; set; }

        [Description(
            "Coefficient to control barrier distance." +
            "Base barrier distance is calculated as sqrt((xppd / (screen-width / horizontal-fov))^2 + ((yppd / (screen-weight / horizontal-fov))^2)")]
        [Category("Coefficients")]
        public double BarrierCoefficient { get; set; }

        [Description("Coefficient to multiply x movement when in barrier")]
        [Category("Coefficients")]
        public double SmallXCoefficient { get; set; }

        [Description("Coefficient to multiply y movement when in barrier")]
        [Category("Coefficients")]
        public double SmallYCoefficient { get; set; }
    }
}
