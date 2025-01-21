using Microsoft.Win32;
using qsslWPF.View;
using qsslWPF.ViewModel;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace qsslWPF.ViewModels
{
    public class MainScreenViewModel : ViewModelBase
    {
        private string _keyPath;

        public string KeyPath
        {
            get
            {
                return _keyPath;
            }
            set
            {
                _keyPath = value;
                OnPropertyChanged(nameof(KeyPath));
            }
        }

        public ICommand OpenKeyCommand { get; }
        public ICommand LoadKeyCommand { get; }
        

        public MainScreenViewModel()
        {
            OpenKeyCommand = new ViewModelCommand(ExecuteOpenKeyCommand);
            LoadKeyCommand = new ViewModelCommand(ExecuteLoadKeyCommand, CanExecuteLoadKeyCommand);
        }

        private bool CanExecuteLoadKeyCommand(object obj)
        {
            bool validData;
            if (string.IsNullOrWhiteSpace(KeyPath))
                validData = false;
            else
                validData = true;

            return validData;
        }

        private void ExecuteLoadKeyCommand(object obj)
        {
            //open login window
            //need to send to client the key
            var loginView = new LoginView();
            loginView.Show();
            loginView.IsVisibleChanged += (s, ev) =>
            {
                if (loginView.IsVisible == false && loginView.IsLoaded)
                {
                    var mainView = new MainScreenView();
                    mainView.Show();
                    loginView.Close();
                }
            };
        }

        private void ExecuteOpenKeyCommand(object obj)
        {
            // Create an instance of OpenFileDialog
            OpenFileDialog openFileDialog = new OpenFileDialog();

            // Set filters for file types (optional)
            openFileDialog.Filter = "Binary files (*.bin)|*.bin|All files (*.*)|*.*";

            // Show the OpenFileDialog
            if (openFileDialog.ShowDialog() == true)
            {
                // Display the selected file path in the TextBox
                KeyPath = openFileDialog.FileName;
            }
        }
    }
}
