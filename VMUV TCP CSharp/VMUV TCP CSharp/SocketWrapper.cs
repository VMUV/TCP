﻿using System;
using System.Net;
using System.Net.Sockets;
using System.Security;
using Trace_Logger_CSharp;
using Comms_Protocol_CSharp;

namespace VMUV_TCP_CSharp
{
    public class SocketWrapper
    {
        private TraceLogger traceLogger = new TraceLogger();
        private Socket listener = null;
        private const int port = 11069;
        private byte[] _dataToSend = new byte[2048];
        private int _numBytesToSend = 0;
        private byte[] _dataReceived = new byte[0];
        private Configuration config;
        private bool clientIsBusy = false;
        private string moduleName = "SocketWrapper.cs";
        private Object _lock = new Object();
        private DataQueue _queue = new DataQueue();

        public const string version = "1.0.2.0";

        public SocketWrapper(Configuration configuration)
        {
            config = configuration;
        }

        public void ServerSetTxData(DataQueue queue)
        {
            lock (_lock)
            {
                _queue.TransferAll(queue);
            }
        }

        public void ClientGetRxData(DataQueue queue)
        {
            lock (_lock)
            {
                queue.TransferAll(_queue);
            }
        }

        public bool ClientHasData()
        {
            bool rtn = false;
            lock (_lock) { rtn = _queue.Count > 0; }
            return rtn;
        }

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

        public TraceLoggerMessage[] GetTraceMessages()
        {
            return traceLogger.GetAllMessages();
        }

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
                Send(handler);
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

        private void Send(Socket handler)
        {
            string methodName = "Send";

            try
            {
                lock (_lock)
                {
                    _numBytesToSend = _queue.GetStreamable(_dataToSend);
                }

                handler.BeginSend(_dataToSend, 0, _numBytesToSend, 0, new AsyncCallback(SendCB), handler);
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
                    byte[] data = new byte[numBytesRead];
                    Buffer.BlockCopy(state.buffer, 0, data, 0, numBytesRead);
                    lock (_lock)
                    {
                        _queue.ParseStreamable(data, data.Length);
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

    public enum Configuration
    {
        server,
        client
    }
}
