using System;
using System.Threading;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VMUV_TCP_CSharp;
using Comms_Protocol_CSharp;

namespace VMUV_TCP_CSharp_Tests
{
    [TestClass]
    public class SocketWrapperTests
    {
        [TestMethod]
        public void SocketWrapper_TestEndToEnd()
        {
            SocketWrapper server = new SocketWrapper(Configuration.server);
            SocketWrapper client = new SocketWrapper(Configuration.client);

            DataQueue knownData = new DataQueue();
            DataQueue toSend = new DataQueue();
            DataQueue toRecieve = new DataQueue();
            short[] known = new short[] { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
            byte[] payload = new byte[known.Length * 2];
            Buffer.BlockCopy(known, 0, payload, 0, payload.Length);
            Motus_1_RawDataPacket dummyPacket = new Motus_1_RawDataPacket(payload);
            for (int i = 0; i < knownData.MaxSize; i++)
            {
                knownData.Add(dummyPacket);
                toSend.Add(dummyPacket);
            }

            server.StartServer();
            server.ServerSetTxData(toSend);
            Assert.IsTrue(toSend.IsEmpty());

            client.ClientStartRead();
            Thread.Sleep(1000);
            Assert.IsTrue(client.ClientHasData());
            client.ClientGetRxData(toRecieve);
            Assert.AreEqual(toRecieve.Count, toRecieve.MaxSize);
            while (!toRecieve.IsEmpty())
            {
                Motus_1_RawDataPacket packetKnown = new Motus_1_RawDataPacket(knownData.Get());
                Motus_1_RawDataPacket packetTest = new Motus_1_RawDataPacket(toRecieve.Get());
                short[] payloadKnown = packetKnown.DeSerialize();
                short[] payloadTest = packetTest.DeSerialize();
                for (int i = 0; i < payloadKnown.Length; i++)
                {
                    Assert.AreEqual(known[i], payloadKnown[i]);
                    Assert.AreEqual(payloadKnown[i], payloadTest[i]);
                }
            }
        }
    }
}
