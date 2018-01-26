using System;
using System.Threading;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VMUV_TCP_CSharp;
using Trace_Logger_CSharp;

namespace VMUV_TCP_CSharp_Tests
{
    [TestClass]
    public class SocketWrapperTests
    {
        [TestMethod]
        public void SocketWrapperEndToEndTest()
        {
            SocketWrapper server = new SocketWrapper(Configuration.server);
            SocketWrapper client = new SocketWrapper(Configuration.client);

            client.StartServer();
            Assert.AreEqual(client.HasTraceMessages(), false);

            server.StartServer();
            Assert.AreEqual(server.HasTraceMessages(), true);
            TraceLoggerMessage[] messages = server.GetTraceMessages();
            Assert.AreEqual(messages.Length, 1);
            Assert.AreEqual(messages[0].moduleName, "SocketWrapper.cs");
            Assert.AreEqual(messages[0].methodName, "StartServer");
            Assert.AreEqual(messages[0].message, "TCP Server successfully started on port 11069");

            // set tx data
            byte[] txData = new byte[] { 0x69, 0x02, 0x45, 0x89 };
            for (int j = 0; j < 10; j++)
            {
                server.ServerSetTxData(txData, 0);
                client.ClientStartRead();

                Assert.AreEqual(false, client.HasTraceMessages());
                Assert.AreEqual(false, server.HasTraceMessages());

                Thread.Sleep(10);

                byte[] rxData = client.ClientGetRxData();
                Assert.AreEqual(txData.Length, rxData.Length);
                byte type = client.ClientGetRxType();
                Assert.AreEqual(0, type);
                for (int i = 0; i < txData.Length; i++)
                    Assert.AreEqual(txData[i], rxData[i]);

                Assert.AreEqual(false, client.HasTraceMessages());
                Assert.AreEqual(false, server.HasTraceMessages());
            }
        }
    }
}
