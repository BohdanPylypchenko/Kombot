using System.ComponentModel;

namespace KombotUI
{
    [TypeConverter(typeof(ExpandableObjectConverter))]
    internal class BgrColor
    {
        public int Blue { get; set; }
        public int Green { get; set; }
        public int Red { get; set; }

        public BgrColor(int blue, int green, int red)
        {
            Blue = blue;
            Green = green;
            Red = red;
        }

        public override string ToString()
        {
            return $"B:{Blue} G:{Green} R:{Red}";
        }
    }
}
