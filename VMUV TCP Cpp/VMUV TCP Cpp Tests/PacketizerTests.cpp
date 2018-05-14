#include "stdafx.h"
#include "CppUnitTest.h"
#include "../VMUV TCP Cpp/Packetizer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace VMUV_TCP_Cpp;

namespace VMUV_TCP_CppTests
{
	TEST_CLASS(PacketizerTests)
	{
	public:
		Packetizer testPacketizer;

		TEST_METHOD(PacketizeEmptyPayload)
		{
			vector<byte> payload;
			vector<byte> packet = testPacketizer.PacketizeData(payload, 0);

			Assert::AreEqual(packet[testPacketizer.sycn1Loc], testPacketizer.sync1);
			Assert::AreEqual(packet[testPacketizer.sycn2Loc], (byte)testPacketizer.sync2);
			Assert::AreEqual(packet[testPacketizer.typeLoc], (byte)0);
			Assert::AreEqual(packet[testPacketizer.lenMSBLoc], (byte)0);
			Assert::AreEqual(packet[testPacketizer.lenLSBLoc], (byte)0);
		}

		// Q: is there a way to test private functions?
		//TEST_METHOD(BuildHeader)
		//{
		//	vector<byte> packet;
		//	short len = numOverHeadBytes + 10;
		//	byte type = 0x12;
		//	testPacketizer.BuildHeader(packet, type, len);

		//	packet[sycn1Loc] = sync1;
		//	packet[sycn2Loc] = sync2;
		//	packet[typeLoc] = type;
		//	packet[lenMSBLoc] = (byte)((len >> 8) & 0xff);
		//	packet[lenLSBLoc] = (byte)(len & 0xff);
		//}
		//TEST_METHOD(CalculateCheckSumFromPayload)
		//{
		//	// todo
		//}
		TEST_METHOD(GetPacketType)
		{
			// todo
		}

		TEST_METHOD(PacketizeIllegalLength)
		{
			bool threwException = false;
			vector<byte> payload(Int16MaxValue + 1);
			try {
				vector<byte> packet = testPacketizer.PacketizeData(payload, 0);
			}
			catch (ArgumentOutOfRangeException &err) {
				threwException = true;
			}
			//catch (...) {
			//	threwException = true;
			//}
			Assert::IsTrue(threwException);
		}

		TEST_METHOD(PacketizeData)
		{
			vector<byte> payload(21);
			strcpy((char*)&payload[0], "this is a test - 123");
			byte type = 0x12;
			short len = (short)(payload.size() & 0xffff);
			short chkSum = 0;

			if (payload.size() >= len)
			{
				for (short i = 0; i < len; i++)
					chkSum += (short)(payload[i] & 0xff);
			}

			vector<byte> packet;
			packet = testPacketizer.PacketizeData(payload, type);

			Assert::AreEqual(packet[testPacketizer.sycn1Loc], testPacketizer.sync1);
			Assert::AreEqual(packet[testPacketizer.sycn2Loc], (byte)testPacketizer.sync2);
			Assert::AreEqual(packet[testPacketizer.typeLoc], type);
			Assert::AreEqual(packet[testPacketizer.lenMSBLoc], (byte)((len >> 8) & 0xff));
			Assert::AreEqual(packet[testPacketizer.lenLSBLoc], (byte)(len & 0xff));
			for (short i = 0; i < len; i++)
			{
				Assert::AreEqual(packet[5 + i], payload[i]);
			}
			Assert::AreEqual(packet[5 + len], (byte)((chkSum >> 8) & 0xff));
			Assert::AreEqual(packet[6 + len], (byte)(chkSum & 0xff));
		}

		TEST_METHOD(IsValidPacketNullPacketTest)
		{
			vector<byte> packet;
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));
		}

		TEST_METHOD(IsValidPacketBadHeader)
		{
			vector<byte> payload(11);
			strcpy((char*)&payload[0], "0123456789");
			vector<byte> packet;
			packet = testPacketizer.PacketizeData(payload, 0);

			// sync1 test
			packet[testPacketizer.sycn1Loc] = testPacketizer.sync2;
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));

			// sync2 test
			packet = testPacketizer.PacketizeData(payload, 0);
			packet[testPacketizer.sycn2Loc] = testPacketizer.sync1;
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));
		}

		TEST_METHOD(IsValidPacketBadLen)
		{
			vector<byte> payload(11);
			strcpy((char*)&payload[0], "0123456789");
			vector<byte> packet;
			packet = testPacketizer.PacketizeData(payload, 0);

			// test illegal length
			packet[testPacketizer.lenMSBLoc] = (byte)((ShortMaxValue + 1) >> 8);
			packet[testPacketizer.lenLSBLoc] = (byte)((ShortMaxValue + 1) & 0xFF);
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));

			// test length too short
			packet[testPacketizer.lenMSBLoc] = (byte)((4) >> 8);
			packet[testPacketizer.lenLSBLoc] = (byte)((4) & 0xFF);
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));

			// test length too long
			packet[testPacketizer.lenMSBLoc] = (byte)((15) >> 8);
			packet[testPacketizer.lenLSBLoc] = (byte)((15) & 0xFF);
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));
		}

		TEST_METHOD(IsValidPacketBadCheckSum)
		{
			vector<byte> payload(11);
			strcpy((char*)&payload[0], "0123456789");
			vector<byte> packet;
			packet = testPacketizer.PacketizeData(payload, 0);

			packet[testPacketizer.dataStartLoc + payload.size()] = 0;
			packet[testPacketizer.dataStartLoc + payload.size() + 1] = 0;
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));
		}

		TEST_METHOD(GetTypeInvalidPacket)
		{
			vector<byte> payload(11);
			strcpy((char*)&payload[0], "0123456789");
			vector<byte> packet;
			packet = testPacketizer.PacketizeData(payload, 0);

			// make packet invalid
			// sync1 test
			packet[testPacketizer.sycn1Loc] = testPacketizer.sync2;

			bool threwException = false;
			try {
				testPacketizer.GetPacketType(packet);
			}
			catch (ArgumentException &err) {
				threwException = true;
			}
			Assert::IsTrue(threwException);
		}

		TEST_METHOD(UnpackDataInvalidPacket)
		{
			vector<byte> payload(11);
			strcpy((char*)&payload[0], "0123456789");
			vector<byte> packet;
			packet = testPacketizer.PacketizeData(payload, 0);

			// make packet invalid
			// sync1 test
			packet[testPacketizer.sycn1Loc] = testPacketizer.sync2;

			bool threwException = false;
			try {
				testPacketizer.UnpackData(packet);
			}
			catch (ArgumentException &err) {
				threwException = true;
			}
			Assert::IsTrue(threwException);
		}

		TEST_METHOD(UnpackValidPacket)
		{
			vector<byte> payload(3);
			payload[0] = (byte)0x56;
			payload[1] = (byte)0x23;
			payload[2] = (byte)0x89;
			vector<byte> packet;
			packet = testPacketizer.PacketizeData(payload, (byte)0x12);

			vector<byte> rtn = testPacketizer.UnpackData(packet);

			for (short i = 0; i < payload.size(); i++)
				Assert::AreEqual(payload[i], rtn[i]);

			byte type = testPacketizer.GetPacketType(packet);
			Assert::AreEqual((byte)0x12, type);
		}
	};
}
