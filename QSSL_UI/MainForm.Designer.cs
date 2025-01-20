
namespace QSSL_UI
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
            this.loadBtn = new System.Windows.Forms.Button();
            this.headerLabel = new System.Windows.Forms.Label();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.keyTextBox = new System.Windows.Forms.TextBox();
            this.openKeyFileBtn = new System.Windows.Forms.Button();
            this.openKeyPanel = new System.Windows.Forms.Panel();
            this.openKeyPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // button1
            // 
            this.loadBtn.Location = new System.Drawing.Point(193, 131);
            this.loadBtn.Name = "button1";
            this.loadBtn.Size = new System.Drawing.Size(96, 29);
            this.loadBtn.TabIndex = 0;
            this.loadBtn.Text = "Load Key";
            this.loadBtn.UseVisualStyleBackColor = true;
            this.loadBtn.Click += new System.EventHandler(this.loadKeybtn_Click);
            // 
            // label1
            // 
            this.headerLabel.AutoSize = true;
            this.headerLabel.Font = new System.Drawing.Font("Arial Black", 30F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(177)));
            this.headerLabel.Location = new System.Drawing.Point(126, 9);
            this.headerLabel.Name = "label1";
            this.headerLabel.Size = new System.Drawing.Size(235, 46);
            this.headerLabel.TabIndex = 1;
            this.headerLabel.Text = "QSSL Bank";
            this.headerLabel.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            this.headerLabel.UseMnemonic = false;
            this.headerLabel.Click += new System.EventHandler(this.label1_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            this.openFileDialog1.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog1_FileOk);
            // 
            // textBox1
            // 
            this.keyTextBox.Location = new System.Drawing.Point(11, 8);
            this.keyTextBox.Name = "textBox1";
            this.keyTextBox.ReadOnly = true;
            this.keyTextBox.Size = new System.Drawing.Size(339, 20);
            this.keyTextBox.TabIndex = 2;
            this.keyTextBox.TextChanged += new System.EventHandler(this.txtFilePath_TextChanged);
            // 
            // openKeyFile
            // 
            this.openKeyFileBtn.Location = new System.Drawing.Point(356, 3);
            this.openKeyFileBtn.Name = "openKeyFile";
            this.openKeyFileBtn.Size = new System.Drawing.Size(96, 29);
            this.openKeyFileBtn.TabIndex = 3;
            this.openKeyFileBtn.Text = "Open Key";
            this.openKeyFileBtn.UseVisualStyleBackColor = true;
            this.openKeyFileBtn.Click += new System.EventHandler(this.openKey_Click);
            // 
            // panel1
            // 
            this.openKeyPanel.Controls.Add(this.openKeyFileBtn);
            this.openKeyPanel.Controls.Add(this.keyTextBox);
            this.openKeyPanel.Location = new System.Drawing.Point(0, 74);
            this.openKeyPanel.Name = "panel1";
            this.openKeyPanel.Size = new System.Drawing.Size(477, 40);
            this.openKeyPanel.TabIndex = 5;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.MenuHighlight;
            this.ClientSize = new System.Drawing.Size(476, 211);
            this.Controls.Add(this.openKeyPanel);
            this.Controls.Add(this.headerLabel);
            this.Controls.Add(this.loadBtn);
            this.Name = "MainForm";
            this.Text = "Bank Application";
            this.Load += new System.EventHandler(this.Main_Load);
            this.openKeyPanel.ResumeLayout(false);
            this.openKeyPanel.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button loadBtn;
        private System.Windows.Forms.Label headerLabel;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.TextBox keyTextBox;
        private System.Windows.Forms.Button openKeyFileBtn;
        private System.Windows.Forms.Panel openKeyPanel;
    }
}

