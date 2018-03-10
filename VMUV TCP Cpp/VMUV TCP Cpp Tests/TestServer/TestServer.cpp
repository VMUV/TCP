// TestServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../VMUV TCP Cpp/SocketWrapper.h"


int main()
{
	int errorCount = 0;
	VMUV_TCP_Cpp::SocketWrapper testServer(VMUV_TCP_Cpp::server);

	// set tx data
	vector<char> txData(4);
	txData[0] = (char)0x69;
	txData[1] = (char)0x02;
	txData[2] = (char)0x45;
	txData[3] = (char)0x89;
	testServer.ServerSetTxData(txData, 0);

	printf("Starting Server\n");
	testServer.StartServer();

	if (!testServer.HasTraceMessages()) {
		printf("   ERROR: testServer.HasTraceMessages() returns false\n");
		++errorCount;
	}
	vector<Trace_Logger_Cpp::TraceLoggerMessage> messages = testServer.GetTraceMessages();
	if (messages.size() != 1) {
		printf("   ERROR: testServer.GetTraceMessages() returned list doesn't have 1 entry\n");
		++errorCount;
	}
	if (messages[0].moduleName != "SocketWrapper.cpp") {
		printf("   ERROR: testServer unexpected message moduleName\n");
		++errorCount;
	}
	if (messages[0].methodName != "StartServer") {
		printf("   ERROR: testServer unexpected message methodName\n");
		++errorCount;
	}
	if (messages[0].message != "TCP Server successfully started on port 11069") {
		printf("   ERROR: testServer unexpected message message\n");
		++errorCount;
	}

	testServer.ContinueServer();

	if (errorCount == 0) {
		printf("   No Errors\n");
	}
	printf("Server Done\n");
	return 0;
}

