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

		TEST_METHOD(ServerSetTxDataTest)
		{
			SocketWrapper testServer(server);
			vector<byte> payload(21);
			strcpy((char*)&payload[0], "this is a test - 123");
			byte type = 0x12;

			testServer.ServerSetTxData(payload, type);
			// need to access private member usePing, txDataPing, txDataPong to verify this function
		}

		TEST_METHOD(ClientGetRxDataTest)
		{
			SocketWrapper testServer(client);
			vector<byte> payload;
			payload = testServer.ClientGetRxData();
			// need to access private members usePing, rxDataPing, rxDataPong to verify this function
		}

		TEST_METHOD(ClientGetRxTypeTest)
		{
			SocketWrapper testServer(client);
			byte type = testServer.ClientGetRxType();
			// need to access private members usePing, rxTypePing, rxTypePong to verify this function
		}

		// note: for this test to succeed, you must first start TestServer executable, 
		// built as a separate project in this solution
		TEST_METHOD(ClientReadTest)
		{
			SocketWrapper testClient(client);

// this is my first attempt to start the test server here
// it didnt work - will try more later...-bd
//			// start the server as a new process
//			STARTUPINFO si;
//			PROCESS_INFORMATION pi;
//
//			ZeroMemory(&si, sizeof(si));
//			si.cb = sizeof(si);
//			ZeroMemory(&pi, sizeof(pi));
//
//			LPCTSTR servername;
//			servername = (LPCTSTR)"TestServer.exe";
//			bool rc = CreateProcess(servername,   // module name
//				NULL,        // Command line
//				NULL,           // Process handle not inheritable
//				NULL,           // Thread handle not inheritable
//				FALSE,          // Set handle inheritance to FALSE
//				0,              // No creation flags
//				NULL,           // Use parent's environment block
//				NULL,           // Use parent's starting directory 
//				&si,            // Pointer to STARTUPINFO structure
//				&pi);           // Pointer to PROCESS_INFORMATION structure
//			Assert::AreEqual(!rc, false);
//
//			Sleep(100);

			// set tx data
			vector<byte> txData(4);
			txData[0] = (byte)0x69;
			txData[1] = (byte)0x02;
			txData[2] = (byte)0x45;
			txData[3] = (byte)0x89;

			testClient.ClientStartRead();
			testClient.ClientContinueRead();
			
//			// Wait until child process exits.
//			WaitForSingleObject(pi.hProcess, INFINITE);
//
//			// Close process and thread handles. 
//			CloseHandle(pi.hProcess);
//			CloseHandle(pi.hThread);

			Assert::AreEqual(testClient.HasTraceMessages(), false);
			vector<byte> rxData = testClient.ClientGetRxData();
			Assert::AreEqual(txData.size() != rxData.size(), false);
			byte type = testClient.ClientGetRxType();
			Assert::AreEqual(type != 0, false);
			for (int i = 0; i < txData.size(); i++) {
				Assert::AreEqual(txData[i] != rxData[i], false);
			}
		}

		// the following test is not finished yet -bd
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