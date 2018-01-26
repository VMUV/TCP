using System;
using System.Net;
using System.Net.Sockets;
using System.Security;
using Trace_Logger_CSharp;

namespace VMUV_TCP_CSharp
{
    public class SocketWrapper
    {
        private Packetizer packetizer = new Packetizer();
        private TraceLogger traceLogger = new TraceLogger();
        private Socket listener = null;
        private const int port = 11069;
        private byte[] txDataPing = { 0 };  // Do this incase Start() is called before the user sets any data
        private byte[] txDataPong = { 0 };  // Do this incase Start() is called before the user sets any data
        private byte[] rxDataPing = { 0 };  // Do this incase GetRxData() is called before the user gets any data
        private byte[] rxDataPong = { 0 };  // Do this incase GetRxData() is called before the user gets any data
        private byte rxTypePing;
        private byte rxTypePong;
        private bool usePing = true;
        private Configuration config;
        private bool clientIsBusy = false;
        private string moduleName = "SocketWrapper.cs";
        private int numPacketsRead = 0;

        /// <summary>
        /// Version number of the current release.
        /// </summary>
        public const string version = "1.1.1";
        /// <summary>
        /// Instantiates a new instance of <c>SocketWrapper</c> configured as either a client or a server.
        /// </summary>
        /// <param name="configuration"></param>
        public SocketWrapper(Configuration configuration)
        {
            config = configuration;
        }

        /// <summary>
        /// Sets the data from <c>payload</c> into the transmit data buffer.
        /// </summary>
        /// <param name="payload"></param>
        /// <param name="type"></param>
        public void ServerSetTxData(byte[] payload, byte type)
        {
            string methodName = "ServerSetTxData";
            try
            {
                if (usePing)
                {
                    txDataPing = packetizer.PacketizeData(payload, (byte)type);
                    usePing = false;
                }
                else
                {
                    txDataPong = packetizer.PacketizeData(payload, (byte)type);
                    usePing = true;
                }
            }
            catch (ArgumentOutOfRangeException e0)
            {
                string msg = e0.Message + e0.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
        }

        /// <summary>
        /// Acquires the most recently received valid data payload.
        /// </summary>
        /// <returns>byte buffer with a copy of the most recently receieved valid data payload.</returns>
        public byte[] ClientGetRxData()
        {
            if (usePing)
                return rxDataPong;
            else
                return rxDataPing;
        }

        /// <summary>
        /// Acquires the most recently received payload type.
        /// </summary>
        /// <returns>byte with the most recent payload type. </returns>
        public byte ClientGetRxType()
        {
            if (usePing)
                return rxTypePong;
            else
                return rxTypePing;
        }

        /// <summary>
        /// Call this method only once after instantiation of the <c>SocketWrapper</c> object. This will start the 
        /// server listener for incoming connections.
        /// </summary>
        public void StartServer()
        {
            string methodName = "StartServer";

            if (config != Configuration.server)
                return;

            try
            {
                IPEndPoint localEP = new IPEndPoint(IPAddress.Loopback, port);
                string msg = "TCP Server successfully started on port " + port.ToString();

                listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                listener.Bind(localEP);
                listener.Listen(100);
                listener.BeginAccept(new AsyncCallback(AcceptCB), listener);

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
            }
            catch (ArgumentNullException e0)
            {
                string msg = e0.Message + e0.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ArgumentOutOfRangeException e1)
            {
                string msg = e1.Message + e1.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (SocketException e2)
            {
                string msg = e2.Message + e2.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ObjectDisposedException e3)
            {
                string msg = e3.Message + e3.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (SecurityException e4)
            {
                string msg = e4.Message + e4.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (NotSupportedException e5)
            {
                string msg = e5.Message + e5.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (InvalidOperationException e6)
            {
                string msg = e6.Message + e6.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (Exception e7)
            {
                string msg = e7.Message + e7.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
        }

        /// <summary>
        /// Call this method in from the main thread to start the next client read process.
        /// </summary>
        public void ClientStartRead()
        {
            string methodName = "ClientStartRead";

            if (clientIsBusy || (config != Configuration.client))
                return;

            try
            {
                IPEndPoint remoteEP = new IPEndPoint(IPAddress.Loopback, port);
                clientIsBusy = true;
                Socket client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                client.BeginConnect(remoteEP, new AsyncCallback(ConnectCB), client);
                return;
            }
            catch (ArgumentNullException e0)
            {
                string msg = e0.Message + e0.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ArgumentOutOfRangeException e1)
            {
                string msg = e1.Message + e1.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (SocketException e2)
            {
                string msg = e2.Message + e2.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ObjectDisposedException e3)
            {
                string msg = e3.Message + e3.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (SecurityException e4)
            {
                string msg = e4.Message + e4.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (InvalidOperationException e6)
            {
                string msg = e6.Message + e6.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (Exception e7)
            {
                string msg = e7.Message + e7.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            finally
            {
                clientIsBusy = false;
            }
        }

        /// <summary>
        /// Returns all stored trace messages within the <c>TraceLogger</c> object. Call this method after first determining if the 
        /// <c>TraceLogger</c> object has any stored messages using the <c>HasTraceMessages</c> method.
        /// </summary>
        /// <returns>A list of <c>TraceLoggerMessage</c> elements.</returns>
        public TraceLoggerMessage[] GetTraceMessages()
        {
            return traceLogger.GetAllMessages();
        }

        /// <summary>
        /// Returns true if there are unread messages stored in the <c>TraceLogger</c> object.
        /// </summary>
        /// <returns>True if unread messages are available. False otherwise.</returns>
        public bool HasTraceMessages()
        {
            return traceLogger.HasMessages();
        }

        private void AcceptCB(IAsyncResult ar)
        {
            string methodName = "AcceptCB";

            try
            {
                Socket local = (Socket)ar.AsyncState;
                Socket handler = listener.EndAccept(ar);

                if (usePing)
                    Send(handler, txDataPong);
                else
                    Send(handler, txDataPing);
            }
            catch (ArgumentNullException e0)
            {
                string msg = e0.Message + e0.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ArgumentOutOfRangeException e1)
            {
                string msg = e1.Message + e1.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (SocketException e2)
            {
                string msg = e2.Message + e2.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ObjectDisposedException e3)
            {
                string msg = e3.Message + e3.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (NotSupportedException e5)
            {
                string msg = e5.Message + e5.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (InvalidOperationException e6)
            {
                string msg = e6.Message + e6.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (Exception e7)
            {
                string msg = e7.Message + e7.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
        }

        private void Send(Socket handler, byte[] data)
        {
            string methodName = "Send";

            try
            {
                handler.BeginSend(data, 0, data.Length, 0, new AsyncCallback(SendCB), handler);
            }
            catch (ArgumentNullException e0)
            {
                string msg = e0.Message + e0.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ArgumentOutOfRangeException e1)
            {
                string msg = e1.Message + e1.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (SocketException e2)
            {
                string msg = e2.Message + e2.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ObjectDisposedException e3)
            {
                string msg = e3.Message + e3.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (Exception e4)
            {
                string msg = e4.Message + e4.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
        }

        private void SendCB(IAsyncResult ar)
        {
            string methodName = "SendCB";

            try
            {
                Socket handler = (Socket)ar.AsyncState;
                int numBytesSent = handler.EndSend(ar);

                handler.Shutdown(SocketShutdown.Both);
                handler.Close();
            }
            catch (SocketException e2)
            {
                string msg = e2.Message + e2.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ObjectDisposedException e3)
            {
                string msg = e3.Message + e3.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (Exception e4)
            {
                string msg = e4.Message + e4.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }

            ResetServer();
        }

        private void ResetServer()
        {
            string methodName = "ResetServer";

            try
            {
                listener.Listen(100);
                listener.BeginAccept(new AsyncCallback(AcceptCB), listener);
            }
            catch (ArgumentOutOfRangeException e1)
            {
                string msg = e1.Message + e1.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (SocketException e2)
            {
                string msg = e2.Message + e2.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ObjectDisposedException e3)
            {
                string msg = e3.Message + e3.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (NotSupportedException e5)
            {
                string msg = e5.Message + e5.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (InvalidOperationException e6)
            {
                string msg = e6.Message + e6.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (Exception e7)
            {
                string msg = e7.Message + e7.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
        }

        private void ConnectCB(IAsyncResult ar)
        {
            string methodName = "ConnectCB";
            try
            {
                Socket client = (Socket)ar.AsyncState;

                client.EndConnect(ar);
                Read(client);
            }
            catch (ArgumentNullException e1)
            {
                string msg = e1.Message + e1.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (SocketException e2)
            {
                string msg = e2.Message + e2.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ObjectDisposedException e3)
            {
                string msg = e3.Message + e3.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ArgumentException e5)
            {
                string msg = e5.Message + e5.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (InvalidOperationException e6)
            {
                string msg = e6.Message + e6.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (Exception e7)
            {
                string msg = e7.Message + e7.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            finally
            {
                clientIsBusy = false;
            }
        }

        private void Read(Socket client)
        {
            StateObject state = new StateObject();
            string methodName = "Read";

            try
            {
                state.workSocket = client;
                client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0, new AsyncCallback(ReadCB), state);
            }
            catch (ArgumentNullException e1)
            {
                string msg = e1.Message + e1.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (SocketException e2)
            {
                string msg = e2.Message + e2.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ObjectDisposedException e3)
            {
                string msg = e3.Message + e3.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ArgumentOutOfRangeException e5)
            {
                string msg = e5.Message + e5.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (Exception e7)
            {
                string msg = e7.Message + e7.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            finally
            {
                clientIsBusy = false;
            }
        }

        private void ReadCB(IAsyncResult ar)
        {
            string methodName = "ReadCB";

            try
            {
                StateObject state = (StateObject)ar.AsyncState;
                Socket client = state.workSocket;
                int numBytesRead = client.EndReceive(ar);

                if (numBytesRead > 0)
                {
                    if (state.packetizer.IsPacketValid(state.buffer))
                    {
                        if (usePing)
                        {
                            rxDataPing = state.packetizer.UnpackData(state.buffer);
                            rxTypePing = state.packetizer.GetPacketType(state.buffer);
                            usePing = false;
                        }
                        else
                        {
                            rxDataPong = state.packetizer.UnpackData(state.buffer);
                            rxTypePong = state.packetizer.GetPacketType(state.buffer);
                            usePing = true;
                        }
                        
                        numPacketsRead++;
                        DebugPrint(numPacketsRead.ToString());
                    }
                }
            }
            catch (ArgumentNullException e1)
            {
                string msg = e1.Message + e1.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (SocketException e2)
            {
                string msg = e2.Message + e2.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ObjectDisposedException e3)
            {
                string msg = e3.Message + e3.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (ArgumentException e5)
            {
                string msg = e5.Message + e5.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (InvalidOperationException e6)
            {
                string msg = e6.Message + e6.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }
            catch (Exception e7)
            {
                string msg = e7.Message + e7.StackTrace;

                traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
                DebugPrint(msg);
            }

            clientIsBusy = false;
        }

        private void DebugPrint(string s)
        {
#if DEBUG
            Console.WriteLine(s);
#endif
        }
    }

    /// <summary>
    /// The possible configurations of the <c>SocketWrapper</c> instance.
    /// </summary>
    public enum Configuration
    {
        server,
        client
    }
}
