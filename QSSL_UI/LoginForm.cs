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
    public partial class LoginForm : Form
    {
        public LoginForm()
        {
            InitializeComponent();
        }

        private void LoginForm_Load(object sender, EventArgs e)
        {
            // Optional: Center the form on the screen
            this.StartPosition = FormStartPosition.CenterScreen;
        }

        private void buttonLogin_Click(object sender, EventArgs e)
        {
            string username = textBoxUsername.Text;
            string password = textBoxPassword.Text;

            // Perform your authentication logic here
            if (username == "admin" && password == "password") // Replace with real validation
            {
                MessageBox.Show("Login successful!", "Success", MessageBoxButtons.OK, MessageBoxIcon.Information);

                // Set DialogResult to OK to indicate successful login
                this.DialogResult = DialogResult.OK;

                // Close the LoginForm
                this.Close();
            }
            else
            {
                MessageBox.Show("Invalid username or password.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            // Set DialogResult to Cancel to indicate login was canceled
            this.DialogResult = DialogResult.Cancel;

            // Close the LoginForm
            this.Close();
        }
        private void labelPassword_Click(object sender, EventArgs e)
        {

        }
        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            if (keyData == Keys.Enter)
            {
                // Trigger the login button click when Enter is pressed
                buttonLogin.PerformClick();
                return true; // Indicate the key press was handled
            }

            return base.ProcessCmdKey(ref msg, keyData);
        }
    }
}


