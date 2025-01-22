using qsslSdk;
using qsslWPF.Model;
using qsslWPF.View;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace qsslWPF.ViewModel
{
    public class LoginViewModel : ViewModelBase
    {
        public event Action RequestClose;
        private SDK sdk;
        //Fields
        private string _username;
        private string _passsword;
        private string _errorMessage;
        private bool _isViewVisible = true;

        public LoginViewModel()
        {
            LoginCommand = new ViewModelCommand(ExecuteLoginCommand, CanExecuteLoginCommand);
            sdk = ((App)Application.Current).sdk;
        }
        public LoginViewModel(string err)
        {
            LoginCommand = new ViewModelCommand(ExecuteLoginCommand, CanExecuteLoginCommand);
            sdk = ((App)Application.Current).sdk;
            ErrorMessage = err;
        }

        public string Username
        {
            get
            {
                return _username;
            }
            set
            {
                _username = value;
                OnPropertyChanged(nameof(Username));
            }
        }
        public string Password
        {
            get
            {
                return _passsword;
            }
            set
            {
                _passsword = value;
                OnPropertyChanged(nameof(Password));
            }
        }
        public string ErrorMessage
        {
            get
            {
                return _errorMessage;
            }
            set
            {
                _errorMessage = value;
                OnPropertyChanged(nameof(ErrorMessage));
            }
        }

        public bool IsViewVisible
        {
            get
            {
                return _isViewVisible;
            }
            set
            {
                _isViewVisible = value;
                OnPropertyChanged(nameof(IsViewVisible));
            }
        }

        public ICommand LoginCommand { get; }


        private bool CanExecuteLoginCommand(object obj)
        {
            bool validData;
            if (string.IsNullOrWhiteSpace(Username) || Password == null)
                validData = false;
            else
                validData = true;

            return validData;
        }

        private void ExecuteLoginCommand(object obj)
        {
            
            var userModel = new UserModel
            {
                Username = Username,
                Password = Password
            };

            //open loading window
            var loadingView = new LoadingView();
            loadingView.Show();
            
            // Request the view to close
            RequestClose?.Invoke();
            
            sdk.SendUserModel(userModel);
            System.Diagnostics.Debug.WriteLine("User sent to SDK!");
        }
    }
}
