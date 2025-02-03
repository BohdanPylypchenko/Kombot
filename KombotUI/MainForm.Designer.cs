namespace KombotUI
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.splitContainerRoot = new System.Windows.Forms.SplitContainer();
            this.tableLayoutPanelButtons = new System.Windows.Forms.TableLayoutPanel();
            this.buttonLoad = new System.Windows.Forms.Button();
            this.buttonSave = new System.Windows.Forms.Button();
            this.propertyGridConfig = new System.Windows.Forms.PropertyGrid();
            this.openFileDialogKCD = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialogKCD = new System.Windows.Forms.SaveFileDialog();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerRoot)).BeginInit();
            this.splitContainerRoot.Panel1.SuspendLayout();
            this.splitContainerRoot.Panel2.SuspendLayout();
            this.splitContainerRoot.SuspendLayout();
            this.tableLayoutPanelButtons.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainerRoot
            // 
            this.splitContainerRoot.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainerRoot.Location = new System.Drawing.Point(0, 0);
            this.splitContainerRoot.Name = "splitContainerRoot";
            this.splitContainerRoot.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainerRoot.Panel1
            // 
            this.splitContainerRoot.Panel1.Controls.Add(this.propertyGridConfig);
            // 
            // splitContainerRoot.Panel2
            // 
            this.splitContainerRoot.Panel2.Controls.Add(this.tableLayoutPanelButtons);
            this.splitContainerRoot.Size = new System.Drawing.Size(800, 450);
            this.splitContainerRoot.SplitterDistance = 398;
            this.splitContainerRoot.TabIndex = 0;
            // 
            // tableLayoutPanelButtons
            // 
            this.tableLayoutPanelButtons.ColumnCount = 2;
            this.tableLayoutPanelButtons.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanelButtons.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanelButtons.Controls.Add(this.buttonLoad, 0, 0);
            this.tableLayoutPanelButtons.Controls.Add(this.buttonSave, 1, 0);
            this.tableLayoutPanelButtons.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanelButtons.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanelButtons.Name = "tableLayoutPanelButtons";
            this.tableLayoutPanelButtons.RowCount = 1;
            this.tableLayoutPanelButtons.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanelButtons.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanelButtons.Size = new System.Drawing.Size(800, 48);
            this.tableLayoutPanelButtons.TabIndex = 0;
            // 
            // buttonLoad
            // 
            this.buttonLoad.Dock = System.Windows.Forms.DockStyle.Fill;
            this.buttonLoad.Location = new System.Drawing.Point(3, 3);
            this.buttonLoad.Name = "buttonLoad";
            this.buttonLoad.Size = new System.Drawing.Size(394, 42);
            this.buttonLoad.TabIndex = 0;
            this.buttonLoad.Text = "Load config";
            this.buttonLoad.UseVisualStyleBackColor = true;
            this.buttonLoad.Click += new System.EventHandler(this.buttonLoad_Click);
            // 
            // buttonSave
            // 
            this.buttonSave.Dock = System.Windows.Forms.DockStyle.Fill;
            this.buttonSave.Location = new System.Drawing.Point(403, 3);
            this.buttonSave.Name = "buttonSave";
            this.buttonSave.Size = new System.Drawing.Size(394, 42);
            this.buttonSave.TabIndex = 1;
            this.buttonSave.Text = "Save config";
            this.buttonSave.UseVisualStyleBackColor = true;
            this.buttonSave.Click += new System.EventHandler(this.buttonSave_Click);
            // 
            // propertyGridConfig
            // 
            this.propertyGridConfig.Dock = System.Windows.Forms.DockStyle.Fill;
            this.propertyGridConfig.Location = new System.Drawing.Point(0, 0);
            this.propertyGridConfig.Name = "propertyGridConfig";
            this.propertyGridConfig.Size = new System.Drawing.Size(800, 398);
            this.propertyGridConfig.TabIndex = 0;
            // 
            // openFileDialogKCD
            // 
            this.openFileDialogKCD.DefaultExt = "kcd";
            this.openFileDialogKCD.FileName = "openFileDialog1";
            // 
            // saveFileDialogKCD
            // 
            this.saveFileDialogKCD.DefaultExt = "kcd";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.splitContainerRoot);
            this.Name = "MainForm";
            this.Text = "KombotUI";
            this.splitContainerRoot.Panel1.ResumeLayout(false);
            this.splitContainerRoot.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerRoot)).EndInit();
            this.splitContainerRoot.ResumeLayout(false);
            this.tableLayoutPanelButtons.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainerRoot;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanelButtons;
        private System.Windows.Forms.Button buttonLoad;
        private System.Windows.Forms.Button buttonSave;
        private System.Windows.Forms.PropertyGrid propertyGridConfig;
        private System.Windows.Forms.OpenFileDialog openFileDialogKCD;
        private System.Windows.Forms.SaveFileDialog saveFileDialogKCD;
    }
}

