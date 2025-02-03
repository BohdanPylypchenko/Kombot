using FluentValidation;

namespace KombotUI
{
    internal class KombotConfigSelectedObjectValidator : AbstractValidator<KombotConfigSelectedObject>
    {
        private static readonly int MIN_COLOR = 0;
        private static readonly int MAX_COLOR = 255;
        private static readonly int MIN_FOV = 96;
        private static readonly int MAX_FOV = 160;

        public KombotConfigSelectedObjectValidator()
        {
            RuleFor(config => config.TargetColor)
                .Must(color => IsColorInRange(color));
            RuleFor(config => config.MaxTargetColorDifference)
                .Must(color => IsColorInRange(color));

            RuleFor(config => config.FrameHalfWidthHeightPx)
                .GreaterThan(0);

            RuleFor(config => config.ScreenWidthRelation)
                .GreaterThan(0);
            RuleFor(config => config.ScreenHeightRelation)
                .GreaterThan(0);

            RuleFor(config => config.MouseXPPD)
                .GreaterThan(0);
            RuleFor(config => config.MouseYPPD)
                .GreaterThan(0);

            RuleFor(config => config.HorizontalFov)
                .InclusiveBetween(MIN_FOV, MAX_FOV);

            RuleFor(config => config.BarrierCoefficient)
                .GreaterThanOrEqualTo(0);
            RuleFor(config => config.SmallXCoefficient)
                .GreaterThanOrEqualTo(0);
            RuleFor(config => config.SmallYCoefficient)
                .GreaterThanOrEqualTo(0);
        }

        private static bool IsColorInRange(BgrColor color)
            {
                return
                    MIN_COLOR <= color.Blue && color.Blue <= MAX_COLOR &&
                    MIN_COLOR <= color.Green && color.Green <= MAX_COLOR &&
                    MIN_COLOR <= color.Red && color.Red <= MAX_COLOR;
            }
        }
    }
