// TestClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../VMUV TCP Cpp/SocketWrapper.h"

#define EMULATE_MOTUS

#define TEST_PAD_VALUE 4000
#define TEST_PAD_VALUE_LSB 0xa0
#define TEST_PAD_VALUE_MSB 0x0f

//int main()
int _cdecl main(int argc, char** argv)
{
	bool domotus = false;
	bool dounit = false;
	if (argc == 2) {
		if (*(argv[1]) == 'm') {
			domotus = true;
		}
		else if (*(argv[1]) == 'u') {
			dounit = true;
		}
		else if (*(argv[1]) == '-') {
			if (*(argv[1]+1) == 'm') {
				domotus = true;
			}
			else if (*(argv[1]+1) == 'u') {
				dounit = true;
			}
		}
	}
	if ((!dounit) && (!domotus)) {
		printf("usage: %s -m    motus emulation mode\n", argv[0]);
		printf("       %s -u    unit test mode\n", argv[0]);
		exit(0);
		return 0;
	}

	int errorCount = 0;
	VMUV_TCP_Cpp::SocketWrapper testClient(VMUV_TCP_Cpp::client);

	if (domotus) {
		printf("Starting Client in Motus Emulation Mode\n");
		int movecount = 4;
		int framecount = 0;

		testClient.ClientStartRead();

		// so we will do this for 14 times
		for (int i = 0; i < 14; i++) {
			// set tx data
			vector<byte> txData(18);
			txData[0] = 0; // forward
			txData[1] = 0;
			txData[2] = 0; // forward right
			txData[3] = 0;
			txData[4] = 0; // right
			txData[5] = 0;
			txData[6] = 0; // back right
			txData[7] = 0;
			txData[8] = 0; // back
			txData[9] = 0;
			txData[10] = 0; // back left
			txData[11] = 0;
			txData[12] = 0; // left
			txData[13] = 0;
			txData[14] = 0; // forward left
			txData[15] = 0;
			txData[16] = TEST_PAD_VALUE_LSB; // center pad
			txData[17] = TEST_PAD_VALUE_MSB;

			// we will be counting 90 frames per move
			// 9 moves total, one for each pad
			++framecount;
			if (framecount > 90) {
				framecount = 0;
				if (movecount >= 0 && movecount < 9) {
					txData[movecount * 2] = TEST_PAD_VALUE_LSB;
					txData[(movecount * 2) + 1] = TEST_PAD_VALUE_MSB;
				}
				++movecount;
				if (movecount >= 9) movecount = 0;
			}

			//		testClient.ClientContinueRead();
			testClient.ClientRead();

			vector<byte> rxData = testClient.ClientGetRxData();

			if (txData.size() != rxData.size()) {
				printf("   ERROR: testClient rxData (%d) and txData (%d) different size\n", rxData.size(), txData.size());
				++errorCount;
			}
			byte type = testClient.ClientGetRxType();
			if (type != 0) {
				printf("   ERROR: testClient rxType and txType different\n");
				++errorCount;
			}
			for (int i = 0; i < txData.size(); i++) {
				if (txData[i] != rxData[i]) {
					printf("   ERROR: testClient rxData (%d) and txData (%d) different at %d\n", rxData[i], txData[i], i);
					++errorCount;
					break;
				}
			}
			if (testClient.HasTraceMessages()) {
				printf("   ERROR: testClient HasTraceMessages() returns true (2)\n");
				++errorCount;
			}
		}
		testClient.ClientCleanup();
	}
	else { // not domotus
		printf("Starting Client in Unit Test Mode\n");
		// set tx data
		vector<byte> txData(4);
		txData[0] = (byte)0x69;
		txData[1] = (byte)0x02;
		txData[2] = (byte)0x45;
		txData[3] = (byte)0x89;
		//	for (int j = 0; j < 10; j++)
		//	{
		testClient.ClientStartRead();
		testClient.ClientContinueRead();

		if (testClient.HasTraceMessages()) {
			printf("   ERROR: testClient HasTraceMessages() returns true (1)\n");
			++errorCount;
		}
		vector<byte> rxData = testClient.ClientGetRxData();
		if (txData.size() != rxData.size()) {
			printf("   ERROR: testClient rxData (%d) and txData (%d) different size\n", rxData.size(), txData.size());
			++errorCount;
		}
		byte type = testClient.ClientGetRxType();
		if (type != 0) {
			printf("   ERROR: testClient rxType and txType different\n");
			++errorCount;
		}
		for (int i = 0; i < txData.size(); i++) {
			if (txData[i] != rxData[i]) {
				printf("   ERROR: testClient rxData (%d) and txData (%d) different at %d\n", rxData[i], txData[i], i);
				++errorCount;
				break;
			}
		}
		if (testClient.HasTraceMessages()) {
			printf("   ERROR: testClient HasTraceMessages() returns true (2)\n");
			++errorCount;
		}

		//	}
	}
	if (errorCount == 0) {
		printf("   No Errors\n");
	}

	printf("Client Done\n");
	return 0;
}

