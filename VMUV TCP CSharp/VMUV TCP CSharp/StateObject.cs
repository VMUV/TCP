using System.Net.Sockets;

namespace VMUV_TCP_CSharp
{
    class StateObject
    {
        public Socket workSocket = null;
        public const int BufferSize = 16535;
        public byte[] buffer = new byte[BufferSize];
    }
}
