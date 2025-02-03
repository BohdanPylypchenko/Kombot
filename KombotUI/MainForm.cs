using FluentValidation.Results;
using System;
using System.Text;
using System.Windows.Forms;

namespace KombotUI
{
    public partial class MainForm : Form
    {
        private KombotConfigSelectedObject kombotConfigSelectedObject;
        private KombotConfigSelectedObjectValidator validator;

        private static readonly string MESSAGE_BOX_GENERIC_ERROR_FORMAT =
        "Error message: {0};\nError stack trace: {1};";

        public MainForm()
        {
            InitializeComponent();

            kombotConfigSelectedObject = new KombotConfigSelectedObject();
            validator = new KombotConfigSelectedObjectValidator();

            propertyGridConfig.SelectedObject = kombotConfigSelectedObject;

            kombotConfigSelectedObject.OnOffKeycode = '8';

            kombotConfigSelectedObject.KeycodesNo = new char[] { 'Q', '3' };
            kombotConfigSelectedObject.KeycodesAlways = new char[] { '1', '2', '4', 'E' };
            kombotConfigSelectedObject.KeycodesOnTarget = new char[] { 'R' };

            kombotConfigSelectedObject.MouseTrigger = MouseTriggerButton.Left;

            kombotConfigSelectedObject.TargetColor = new BgrColor(10, 10, 245);
            kombotConfigSelectedObject.MaxTargetColorDifference = new BgrColor(10, 10, 10);

            kombotConfigSelectedObject.FrameHalfWidthHeightPx = 128;
            kombotConfigSelectedObject.ScreenWidthRelation = 16;
            kombotConfigSelectedObject.ScreenHeightRelation = 10;
            kombotConfigSelectedObject.MouseXPPD = 1819;
            kombotConfigSelectedObject.MouseYPPD = 1819;
            kombotConfigSelectedObject.HorizontalFov = 128;
            kombotConfigSelectedObject.BarrierCoefficient = 0.1;
            kombotConfigSelectedObject.SmallXCoefficient = 50;
            kombotConfigSelectedObject.SmallYCoefficient = 50;
        }

        private void buttonLoad_Click(object sender, System.EventArgs e)
        {
            if (openFileDialogKCD.ShowDialog() != DialogResult.OK) return;

            try
            {
                string path = openFileDialogKCD.FileName;

                kombotConfigSelectedObject = ConverterKombotConfigDataSelectedObject
                    .ToSelectedObject(KombotConfigData.FromFile(path), path);

                propertyGridConfig.SelectedObject = kombotConfigSelectedObject;
            }
            catch (Exception ex)
            {
                ShowErrorMessageBox(
                    string.Format(MESSAGE_BOX_GENERIC_ERROR_FORMAT, ex.Message, ex.StackTrace)
                );
            }
        }

        private void buttonSave_Click(object sender, System.EventArgs e)
        {
            if (saveFileDialogKCD.ShowDialog() != DialogResult.OK) return;

            var results = validator.Validate(kombotConfigSelectedObject);
            if (!results.IsValid)
            {
                StringBuilder stringBuilder = new StringBuilder();
                stringBuilder.AppendLine("Experiment parameters are invalid. List of validation errors:");
                foreach (ValidationFailure failure in results.Errors)
                {
                    stringBuilder.AppendLine($"- {failure.ErrorMessage}");
                }

                ShowErrorMessageBox(stringBuilder.ToString());
                return;
            }

            try
            {
                string path = saveFileDialogKCD.FileName;

                ConverterKombotConfigDataSelectedObject
                    .ToConfigData(kombotConfigSelectedObject)
                    .ToFile(path);

                kombotConfigSelectedObject.SetPath(path);

                propertyGridConfig.SelectedObject = kombotConfigSelectedObject;
            }
            catch (Exception ex)
            {
                ShowErrorMessageBox(
                    string.Format(MESSAGE_BOX_GENERIC_ERROR_FORMAT, ex.Message, ex.StackTrace)
                );
            }
        }

        private static void ShowErrorMessageBox(string errorDescription)
        {
            MessageBox.Show(
                errorDescription,
                "Error",
                MessageBoxButtons.OK,
                MessageBoxIcon.Error
            );
        }
    }
}
