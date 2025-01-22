using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using static qsslSdk.Constants;

namespace qsslSdk
{
    class UdpComm
    {
        private Socket _socket;
        private IPEndPoint _localEndPoint;
        private EndPoint _remoteEndPoint;

        public UdpComm()
        {
            _socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            _localEndPoint = new IPEndPoint(IPAddress.Parse(LOCAL_ADDRESS), LOCAL_PORT);
            _socket.Bind(_localEndPoint);

            // Set a remote endpoint placeholder for recvfrom
            _remoteEndPoint = new IPEndPoint(IPAddress.Any, 0);
        }

        private void SendMessage(string remoteAddress, int remotePort, string message)
        {
            _remoteEndPoint = new IPEndPoint(IPAddress.Parse(remoteAddress), remotePort);
            byte[] data = Encoding.UTF8.GetBytes(message);
            _socket.SendTo(data, _remoteEndPoint);
            Console.WriteLine($"Sent: {message} to {remoteAddress}:{remotePort}");
        }

        private string ReceiveMessage()
        {
            byte[] buffer = new byte[1024];
            int receivedBytes = _socket.ReceiveFrom(buffer, ref _remoteEndPoint);
            string message = Encoding.UTF8.GetString(buffer, 0, receivedBytes);
            Console.WriteLine($"Received: {message} from {_remoteEndPoint}");
            return message;
        }


        public void Stop()
        {
            _socket.Close();
        }

        public string SendAndRecv(string message) 
        {
            SendMessage(REMOTE_ADDRESS, REMOTE_PORT, message);
            return ReceiveMessage();
        }
    }

}
