using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VMUV_TCP_CSharp;

namespace VMUV_TCP_CSharp_Tests
{
    [TestClass]
    public class PacketizerTests
    {
        Packetizer packetizer = new Packetizer();

        [TestMethod]
        public void PacketizeNullPayload()
        {
            byte[] packet = packetizer.PacketizeData(null, 0);

            Assert.AreEqual(packet[Packetizer.sycn1Loc], Packetizer.sync1);
            Assert.AreEqual(packet[Packetizer.sycn2Loc], Packetizer.sync2);
            Assert.AreEqual(packet[Packetizer.typeLoc], 0);
            Assert.AreEqual(packet[Packetizer.lenMSBLoc], 0);
            Assert.AreEqual(packet[Packetizer.lenLSBLoc], 0);
        }

        [TestMethod]
        [ExpectedException(typeof(ArgumentOutOfRangeException),"An illegal length was accepted")]
        public void PacketizeIllegalLength()
        {
            byte[] payload = new byte[short.MaxValue + 1];
            byte[] packet = packetizer.PacketizeData(payload, 0);
        }

        [TestMethod]
        public void PacketizeData()
        {
            byte[] payload = new byte[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            byte[] packet = packetizer.PacketizeData(payload, 0);

            Assert.AreEqual(packet[Packetizer.sycn1Loc], Packetizer.sync1);
            Assert.AreEqual(packet[Packetizer.sycn2Loc], Packetizer.sync2);
            Assert.AreEqual(packet[Packetizer.typeLoc], 0);
            Assert.AreEqual(packet[Packetizer.lenMSBLoc], (byte)((payload.Length >> 8) & 0xFF));
            Assert.AreEqual(packet[Packetizer.lenLSBLoc], (byte)(payload.Length & 0xFF));

            for (short i = 0; i < payload.Length; i++) Assert.AreEqual(packet[Packetizer.dataStartLoc + i], payload[i]);
        }

        [TestMethod]
        public void IsValidPacketNullPacketTest()
        {
            Assert.AreEqual(false, packetizer.IsPacketValid(null));
        }

        [TestMethod]
        public void IsValidPacketBadHeader()
        {
            byte[] payload = new byte[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            byte[] packet = packetizer.PacketizeData(payload, 0);

            // sync1 test
            packet[Packetizer.sycn1Loc] = Packetizer.sync2;
            Assert.AreEqual(false, packetizer.IsPacketValid(packet));

            // sync2 test
            packet = packetizer.PacketizeData(payload, 0);
            packet[Packetizer.sycn2Loc] = Packetizer.sync1;
            Assert.AreEqual(false, packetizer.IsPacketValid(packet));
        }

        [TestMethod]
        public void IsValidPacketBadLen()
        {
            byte[] payload = new byte[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            byte[] packet = packetizer.PacketizeData(payload, 0);

            // test illegal length
            packet[Packetizer.lenMSBLoc] = (byte)((short.MaxValue + 1) >> 8);
            packet[Packetizer.lenLSBLoc] = (byte)((short.MaxValue + 1) & 0xFF);
            Assert.AreEqual(false, packetizer.IsPacketValid(packet));

            // test length too short
            packet[Packetizer.lenMSBLoc] = (byte)((4) >> 8);
            packet[Packetizer.lenLSBLoc] = (byte)((4) & 0xFF);
            Assert.AreEqual(false, packetizer.IsPacketValid(packet));

            // test length too long
            packet[Packetizer.lenMSBLoc] = (byte)((15) >> 8);
            packet[Packetizer.lenLSBLoc] = (byte)((15) & 0xFF);
            Assert.AreEqual(false, packetizer.IsPacketValid(packet));
        }

        [TestMethod]
        public void IsValidPacketBadCheckSum()
        {
            byte[] payload = new byte[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            byte[] packet = packetizer.PacketizeData(payload, 0);

            packet[Packetizer.dataStartLoc + payload.Length] = 0;
            packet[Packetizer.dataStartLoc + payload.Length + 1] = 0;
            Assert.AreEqual(false, packetizer.IsPacketValid(packet));
        }

        [TestMethod]
        [ExpectedException(typeof(ArgumentException), 
            "An invalid packet was allowed to return a type")]
        public void GetTypeInvalidPacket()
        {
            packetizer.GetPacketType(null);
        }

        [TestMethod]
        [ExpectedException(typeof(ArgumentException),
            "An invalid packet was allowed to return a payload")]
        public void UnpackDataInvalidPacket()
        {
            packetizer.UnpackData(null);
        }

        [TestMethod]
        public void UnpackValidPacket()
        {
            byte[] payload = new byte[] { 0x56, 0x23, 0x89 };
            byte[] packet = packetizer.PacketizeData(payload, 0x12);

            byte[] rtn = packetizer.UnpackData(packet);
            for (short i = 0; i < payload.Length; i++)
                Assert.AreEqual(payload[i], rtn[i]);

            byte type = packetizer.GetPacketType(packet);
            Assert.AreEqual(0x12, type);
        }
    }
}
