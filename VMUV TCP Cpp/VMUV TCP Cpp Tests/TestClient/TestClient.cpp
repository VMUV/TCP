// TestClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../VMUV TCP Cpp/SocketWrapper.h"


int main()
{
	int errorCount = 0;
	VMUV_TCP_Cpp::SocketWrapper testClient(VMUV_TCP_Cpp::client);

	printf("Starting Client\n");
	// set tx data
	vector<char> txData(4);
	txData[0] = (char)0x69;
	txData[1] = (char)0x02;
	txData[2] = (char)0x45;
	txData[3] = (char)0x89;
//	for (int j = 0; j < 10; j++)
//	{
		testClient.ClientStartRead();
		testClient.ClientContinueRead();

		if (testClient.HasTraceMessages()) {
			printf("   ERROR: testClient HasTraceMessages() returns true (1)\n");
			++errorCount;
		}
		vector<char> rxData = testClient.ClientGetRxData();
		if (txData.size() != rxData.size()) {
			printf("   ERROR: testClient rxData and txData different size\n");
			++errorCount;
		}
		char type = testClient.ClientGetRxType();
		if (type != 0) {
			printf("   ERROR: testClient rxType and txType different\n");
			++errorCount;
		}
		for (int i = 0; i < txData.size(); i++) {
			if (txData[i] != rxData[i]) {
				printf("   ERROR: testClient rxData and txData different\n");
				++errorCount;
				break;
			}
		}
		if (testClient.HasTraceMessages()) {
			printf("   ERROR: testClient HasTraceMessages() returns true (2)\n");
			++errorCount;
		}
		if (errorCount == 0) {
			printf("   No Errors\n");
		}

//	}
	printf("Client Done\n");
	return 0;
}

