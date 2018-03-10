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
			vector<char> payload;
			vector<char> packet = testPacketizer.PacketizeData(payload, 0);

			Assert::AreEqual(packet[testPacketizer.sycn1Loc], testPacketizer.sync1);
			Assert::AreEqual(packet[testPacketizer.sycn2Loc], testPacketizer.sync2);
			Assert::AreEqual(packet[testPacketizer.typeLoc], (char)0);
			Assert::AreEqual(packet[testPacketizer.lenMSBLoc], (char)0);
			Assert::AreEqual(packet[testPacketizer.lenLSBLoc], (char)0);
		}

		// Q: is there a way to test private functions?
		//TEST_METHOD(BuildHeader)
		//{
		//	vector<char> packet;
		//	short len = numOverHeadBytes + 10;
		//	char type = 0x12;
		//	testPacketizer.BuildHeader(packet, type, len);

		//	packet[sycn1Loc] = sync1;
		//	packet[sycn2Loc] = sync2;
		//	packet[typeLoc] = type;
		//	packet[lenMSBLoc] = (char)((len >> 8) & 0xff);
		//	packet[lenLSBLoc] = (char)(len & 0xff);
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
			vector<char> payload(Int16MaxValue + 1);
			try {
				vector<char> packet = testPacketizer.PacketizeData(payload, 0);
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
			vector<char> payload(21);
			strcpy(&payload[0], "this is a test - 123");
			char type = 0x12;
			short len = (short)(payload.size() & 0xffff);
			short chkSum = 0;

			if (payload.size() >= len)
			{
				for (short i = 0; i < len; i++)
					chkSum += (short)(payload[i] & 0xff);
			}

			vector<char> packet;
			packet = testPacketizer.PacketizeData(payload, type);

			Assert::AreEqual(packet[testPacketizer.sycn1Loc], testPacketizer.sync1);
			Assert::AreEqual(packet[testPacketizer.sycn2Loc], testPacketizer.sync2);
			Assert::AreEqual(packet[testPacketizer.typeLoc], type);
			Assert::AreEqual(packet[testPacketizer.lenMSBLoc], (char)((len >> 8) & 0xff));
			Assert::AreEqual(packet[testPacketizer.lenLSBLoc], (char)(len & 0xff));
			for (short i = 0; i < len; i++)
			{
				Assert::AreEqual(packet[5 + i], payload[i]);
			}
			Assert::AreEqual(packet[5 + len], (char)((chkSum >> 8) & 0xff));
			Assert::AreEqual(packet[6 + len], (char)(chkSum & 0xff));
		}

		TEST_METHOD(IsValidPacketNullPacketTest)
		{
			vector<char> packet;
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));
		}

		TEST_METHOD(IsValidPacketBadHeader)
		{
			vector<char> payload(11);
			strcpy(&payload[0], "0123456789");
			vector<char> packet;
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
			vector<char> payload(11);
			strcpy(&payload[0], "0123456789");
			vector<char> packet;
			packet = testPacketizer.PacketizeData(payload, 0);

			// test illegal length
			packet[testPacketizer.lenMSBLoc] = (char)((ShortMaxValue + 1) >> 8);
			packet[testPacketizer.lenLSBLoc] = (char)((ShortMaxValue + 1) & 0xFF);
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));

			// test length too short
			packet[testPacketizer.lenMSBLoc] = (char)((4) >> 8);
			packet[testPacketizer.lenLSBLoc] = (char)((4) & 0xFF);
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));

			// test length too long
			packet[testPacketizer.lenMSBLoc] = (char)((15) >> 8);
			packet[testPacketizer.lenLSBLoc] = (char)((15) & 0xFF);
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));
		}

		TEST_METHOD(IsValidPacketBadCheckSum)
		{
			vector<char> payload(11);
			strcpy(&payload[0], "0123456789");
			vector<char> packet;
			packet = testPacketizer.PacketizeData(payload, 0);

			packet[testPacketizer.dataStartLoc + payload.size()] = 0;
			packet[testPacketizer.dataStartLoc + payload.size() + 1] = 0;
			Assert::AreEqual(false, testPacketizer.IsPacketValid(packet));
		}

		TEST_METHOD(GetTypeInvalidPacket)
		{
			vector<char> payload(11);
			strcpy(&payload[0], "0123456789");
			vector<char> packet;
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
			vector<char> payload(11);
			strcpy(&payload[0], "0123456789");
			vector<char> packet;
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
			vector<char> payload(3);
			payload[0] = (char)0x56;
			payload[1] = (char)0x23;
			payload[2] = (char)0x89;
			vector<char> packet;
			packet = testPacketizer.PacketizeData(payload, (char)0x12);

			vector<char> rtn = testPacketizer.UnpackData(packet);

			for (short i = 0; i < payload.size(); i++)
				Assert::AreEqual(payload[i], rtn[i]);

			char type = testPacketizer.GetPacketType(packet);
			Assert::AreEqual((char)0x12, type);
		}
	};
}
