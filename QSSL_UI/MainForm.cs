using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace QSSL_UI
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        private void Main_Load(object sender, EventArgs e)
        {

        }

        private void loadKeybtn_Click(object sender, EventArgs e)
        {
             ShowLoginForm();
        }


        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void openFileDialog1_FileOk(object sender, CancelEventArgs e)
        {

        }

        private void txtFilePath_TextChanged(object sender, EventArgs e)
        {

        }

        private void openKey_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Filter = "All Files|*.*";
                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {
                    // Display the selected file path in the TextBox
                    keyTextBox.Text = openFileDialog.FileName;
                }
            }
        }

        private void ShowLoginForm()
        {
            // Create and show the LoginForm
            LoginForm loginForm = new LoginForm();
            this.Hide();

            // Show the LoginForm as a dialog
            if (loginForm.ShowDialog() != DialogResult.OK)
            {
                MessageBox.Show("Login canceled or failed.", "Notice");
            }
            else
            {
               
            }
            this.Show();
        }
    }
}
