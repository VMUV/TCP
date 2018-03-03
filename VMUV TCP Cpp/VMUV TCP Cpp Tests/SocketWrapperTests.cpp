#include "stdafx.h"
#include "CppUnitTest.h"
#include "../VMUV TCP Cpp/SocketWrapper.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace VMUV_TCP_Cpp;
//using namespace Trace_Logger_Cpp;

namespace VMUV_TCP_CppTests
{
	TEST_CLASS(SocketWrapperTests)
	{
	public:

//		TEST_METHOD(ServerSetTxDataTest)
//		{
//			SocketWrapper testServer(server);
//			vector<char> payload(21);
//			strcpy(&payload[0], "this is a test - 123");
//			char type = 0x12;
//
//			testServer.ServerSetTxData(payload, type);
//			// need to access private member usePing, txDataPing, txDataPong to verify this function
//		}
//
//		TEST_METHOD(ClientGetRxDataTest)
//		{
//			SocketWrapper testServer(client);
//			vector<char> payload;
//			payload = testServer.ClientGetRxData();
//			// need to access private members usePing, rxDataPing, rxDataPong to verify this function
//		}
//
//		TEST_METHOD(ClientGetRxTypeTest)
//		{
//			SocketWrapper testServer(client);
//			char type = testServer.ClientGetRxType();
//			// need to access private members usePing, rxTypePing, rxTypePong to verify this function
//		}
//
//		TEST_METHOD(StartServerTest)
//		{
//			return;
//			SocketWrapper testServer(server);
////			SocketWrapper testClient(client);
//
//			//client.StartServer();
//			//Assert.AreEqual(testServer.HasTraceMessages(), false);
//
////			testServer.StartServer();
//			// we expect the thread to hang listening for a client
//			testServer.StartServerTestThread();
//			Sleep(10);
//
//			Assert::AreEqual(testServer.serverTestRunning, true);
////			Assert::AreEqual(testServer.HasTraceMessages(), true);
//
//			testServer.StopServerTestThread();
//			Assert::AreEqual(testServer.serverTestRunning, false);
//
////			vector<Trace_Logger_Cpp::TraceLoggerMessage> messages;
////			messages = testServer.GetTraceMessages();
////			Assert::AreEqual((int)messages.size(), (int)1);
////			Assert::AreEqual(&(messages[0].moduleName[0]), "SocketWrapper.cpp");
////			Assert::AreEqual(&(messages[0].methodName[0]), "StartServer");
////			Assert::AreEqual(&(messages[0].message[0]), "TCP Server successfully started on port 11069");
//
//			// ok, so I dont know how to stop the server - perhaps it happens when the SocketWrapper is destroyed?
//
//			//// set tx data
//			//byte[] txData = new byte[]{ 0x69, 0x02, 0x45, 0x89 };
//			//vector<char> txData(4);
//			//txData[0] = (char)0x69;
//			//txData[1] = (char)0x02;
//			//txData[2] = (char)0x45;
//			//txData[3] = (char)0x89;
//			//for (int j = 0; j < 10; j++)
//			//{
//			//	server.ServerSetTxData(txData, 0);
//		}
//
		TEST_METHOD(ClientStartReadTest)
		{
			SocketWrapper testServer(server);
			SocketWrapper testClient(client);

			testServer.StartServerTestThread();
			Sleep(10);

			vector<char> txData(4);
			txData[0] = (char)0x69;
			txData[1] = (char)0x02;
			txData[2] = (char)0x45;
			txData[3] = (char)0x89;
			testServer.ServerSetTxData(txData, 0);

			//			testClient.ClientStartRead();
			// we expect the thread to hang waiting to connect
			// well, perhaps it times out...
			testClient.StartClientTestThread();
			Sleep(100);

			Assert::AreEqual(testClient.clientTestRunning, true);
			testClient.StopClientTestThread();
			Assert::AreEqual(testClient.clientTestRunning, false);
			testServer.StopServerTestThread();
		}
		//TEST_METHOD(GetTraceMessagesTest)
		//{
		//	// todo
		//}

		//TEST_METHOD(HasTraceMessagesTest)
		//{
		//	// todo
		//}

		//TEST_METHOD(AcceptCBTest)
		//{
		//	// todo
		//}

		//TEST_METHOD(SendTest)
		//{
		//	// todo
		//}

		//TEST_METHOD(SendCBTest)
		//{
		//	// todo
		//}

		//TEST_METHOD(ResetServerTest)
		//{
		//	// todo
		//}

		//TEST_METHOD(ConnectCBTest)
		//{
		//	// todo
		//}

		//TEST_METHOD(ReadTest)
		//{
		//	// todo
		//}

		//TEST_METHOD(ReadCBTest)
		//{
		//	// todo
		//}

		//TEST_METHOD(SocketWrapperEndToEndTest)
		//{
		//	//SocketWrapper server = new SocketWrapper(Configuration.server);
		//	//SocketWrapper client = new SocketWrapper(Configuration.client);

		//	//client.StartServer();
		//	//Assert.AreEqual(client.HasTraceMessages(), false);

		//	//server.StartServer();
		//	//Assert.AreEqual(server.HasTraceMessages(), true);
		//	//TraceLoggerMessage[] messages = server.GetTraceMessages();
		//	//Assert.AreEqual(messages.Length, 1);
		//	//Assert.AreEqual(messages[0].moduleName, "SocketWrapper.cs");
		//	//Assert.AreEqual(messages[0].methodName, "StartServer");
		//	//Assert.AreEqual(messages[0].message, "TCP Server successfully started on port 11069");

		//	//// set tx data
		//	//byte[] txData = new byte[]{ 0x69, 0x02, 0x45, 0x89 };
		//	//for (int j = 0; j < 10; j++)
		//	//{
		//	//	server.ServerSetTxData(txData, 0);
		//	//	client.ClientStartRead();

		//	//	Assert.AreEqual(false, client.HasTraceMessages());
		//	//	Assert.AreEqual(false, server.HasTraceMessages());

		//	//	Thread.Sleep(10);

		//	//	byte[] rxData = client.ClientGetRxData();
		//	//	Assert.AreEqual(txData.Length, rxData.Length);
		//	//	byte type = client.ClientGetRxType();
		//	//	Assert.AreEqual(0, type);
		//	//	for (int i = 0; i < txData.Length; i++)
		//	//		Assert.AreEqual(txData[i], rxData[i]);

		//	//	Assert.AreEqual(false, client.HasTraceMessages());
		//	//	Assert.AreEqual(false, server.HasTraceMessages());
		//	//}
		//}

	};
}