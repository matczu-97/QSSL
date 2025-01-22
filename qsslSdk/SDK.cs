using qsslWPF.Model;
using System;

namespace qsslSdk
{
    public class SDK
    {
        private readonly UdpComm udpComm;

        // Event to notify WPF about login results
        public event Action<bool> LoginResultEvent;

        public SDK()
        {
            udpComm = new UdpComm();
        }

        // Send file path to the backend
        public void SendFilePath(string filePath)
        {
            // Simulate processing file path
            System.Diagnostics.Debug.WriteLine($"File path received: {filePath}");
            //udpComm.SendAndRecv(filePath);
            System.Diagnostics.Debug.WriteLine($"File path sent to client.");
        }

        // Send user model (username and password) to the backend
        public void SendUserModel(UserModel userModel)
        {
            // Simulate backend login logic
            bool loginSuccess = userModel.Username == "admin" && userModel.Password == "password";

            // Trigger the login result event
            LoginResultEvent?.Invoke(loginSuccess);
        }
    }

}
