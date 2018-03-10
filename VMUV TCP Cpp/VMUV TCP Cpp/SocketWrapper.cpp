// SocketWrapper.cpp

#include "stdafx.h"

#include "SocketWrapper.h"

WSADATA wsaData;

VMUV_TCP_Cpp::SocketWrapper::SocketWrapper()
{
	ListenSocket = INVALID_SOCKET;
	txDataPing = { 0 };  // Do this incase Start() is called before the user sets any data
	txDataPong = { 0 };  // Do this incase Start() is called before the user sets any data
	rxDataPing = { 0 };  // Do this incase GetRxData() is called before the user gets any data
	rxDataPong = { 0 };  // Do this incase GetRxData() is called before the user gets any data
	usePing = true;
	clientIsBusy = false;
	moduleName = "SocketWrapper.cpp";
	numPacketsRead = 0;
	serverTestRunning = false;
	clientTestRunning = false;
}


		/// <summary>
		/// Instantiates a new instance of <c>SocketWrapper</c> configured as either a client or a server.
		/// </summary>
		/// <param name="configuration"></param>
VMUV_TCP_Cpp::SocketWrapper::SocketWrapper(Configuration configuration)
{
	config = configuration;
	ListenSocket = INVALID_SOCKET;
	txDataPing = { 0 };  // Do this incase Start() is called before the user sets any data
	txDataPong = { 0 };  // Do this incase Start() is called before the user sets any data
	rxDataPing = { 0 };  // Do this incase GetRxData() is called before the user gets any data
	rxDataPong = { 0 };  // Do this incase GetRxData() is called before the user gets any data
	usePing = true;
	clientIsBusy = false;
	moduleName = "SocketWrapper.cpp";
	numPacketsRead = 0;
	serverTestRunning = false;
	clientTestRunning = false;
}

VMUV_TCP_Cpp::SocketWrapper::~SocketWrapper()
{
}

/// <summary>
/// Sets the data from <c>payload</c> into the transmit data buffer.
/// </summary>
/// <param name="payload"></param>
/// <param name="type"></param>
void VMUV_TCP_Cpp::SocketWrapper::ServerSetTxData(vector<char> payload, char type)
{
	string methodName = "ServerSetTxData";
	try
	{
		if (usePing)
		{
			txDataPing = packetizer.PacketizeData(payload, (char)type);
			usePing = false;
		}
		else
		{
			txDataPong = packetizer.PacketizeData(payload, (char)type);
			usePing = true;
		}
	}
	catch (ArgumentOutOfRangeException &e0)
	{
		string msg = e0.Message + e0.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
}

/// <summary>
/// Acquires the most recently received valid data payload.
/// </summary>
/// <returns>char buffer with a copy of the most recently receieved valid data payload.</returns>
vector<char> VMUV_TCP_Cpp::SocketWrapper::ClientGetRxData()
{
	if (usePing)
		return rxDataPong;
	else
		return rxDataPing;
}

/// <summary>
/// Acquires the most recently received payload type.
/// </summary>
/// <returns>char with the most recent payload type. </returns>
char VMUV_TCP_Cpp::SocketWrapper::ClientGetRxType()
{
	if (usePing)
		return rxTypePong;
	else
		return rxTypePing;
}

/// <summary>
/// Call this method only once after instantiation of the <c>SocketWrapper</c> object. This will start the 
/// server listener for incoming connections.
/// </summary>
void VMUV_TCP_Cpp::SocketWrapper::StartServer()
{
	//-------------------------
	// Declare and initialize variables
	string methodName = "StartServer";
	char tmpstr[100];
	char portstr[10];
	char* argv1 = "localhost";

//	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
//	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
//	WSANETWORKEVENTS NetworkEvents;
//	DWORD EventTotal = 0;

//	HANDLE NewEvent = NULL;

	if (config != Configuration::server)
		return;

	try
	{
		// the following from document msdn.microsoft.com ...> Using Winsock > Getting Started With Winsock
		// initializa Winsock
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			throw SocketException("winsock ", "unable to start winsock");
		}

		sprintf_s(portstr, "%d", port);

		struct addrinfo *result = NULL;
		struct addrinfo *ptr = NULL, hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the local address and port to be used by the server
		iResult = getaddrinfo(NULL, portstr, &hints, &result);
		if (iResult != 0) {
			sprintf_s(tmpstr, "getaddrinfo failed: %d\n", iResult);
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}
		
		string msg = "TCP Server successfully started on port " + string(portstr);

		//-------------------------
		// Create a listening socket
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

		if (ListenSocket == INVALID_SOCKET) {
			sprintf_s(tmpstr, "Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Bind the listening socket
		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			sprintf_s(tmpstr, "bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}
		freeaddrinfo(result);

		//-------------------------
		// Start listening on the socket
		if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
			sprintf_s(tmpstr, "Listen failed with error: %ld\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}

		// Accept a handler socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			sprintf_s(tmpstr, "accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}

//		AcceptCB(handler);

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
	}
	catch (ArgumentNullException &e0)
	{
		string msg = e0.Message + e0.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentOutOfRangeException &e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException &e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException &e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SecurityException &e4)
	{
		string msg = e4.Message + e4.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (NotSupportedException &e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException &e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception &e7)
	{
		string msg = e7.Message + e7.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
}


void VMUV_TCP_Cpp::SocketWrapper::ContinueServer()
{
	int iResult;
	int iSendResult;

	// No longer need server socket
	printf("about to invoke closesocket(ListenSocket)\n");
	closesocket(ListenSocket);

	AcceptCB(ClientSocket);

	//// cleanup
	WSACleanup();
	printf("ContinueServer Done\n");
}

/// <summary>
/// Call this method in from the main thread to start the next client read process.
/// </summary>
void VMUV_TCP_Cpp::SocketWrapper::ClientStartRead()
{
	//-------------------------
	// Declare and initialize variables
	string methodName = "ClientStartRead";
	char tmpstr[100];
	char portstr[10];
	char* argv1 = "localhost";

	ConnectSocket = INVALID_SOCKET;
//	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
//	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
//	WSANETWORKEVENTS NetworkEvents;
//	DWORD EventTotal = 0;

//	HANDLE NewEvent = NULL;

	if (clientIsBusy || (config != Configuration::client))
		return;

	try
	{
		// the following from document msdn.microsoft.com ...> Using Winsock > Getting Started With Winsock
		// initializa Winsock
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			throw SocketException("winsock ", "unable to start winsock");
		}

		sprintf_s(portstr, "%d", port);

		struct addrinfo *result = NULL;
		struct addrinfo *ptr = NULL, hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		iResult = getaddrinfo(argv1, portstr, &hints, &result);
		if (iResult != 0) {
			sprintf_s(tmpstr, "getaddrinfo failed: %d\n", iResult);
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}

		clientIsBusy = true;
		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				sprintf_s(tmpstr, "Error at socket(): %ld\n", WSAGetLastError());
				freeaddrinfo(ptr);
				WSACleanup();
				throw SocketException("winsock ", tmpstr);
			}

			// Connect to server.
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			else {
//				ConnectCB(ConnectSocket);
			}
			break;
		}

		freeaddrinfo(result);

		if (ConnectSocket == INVALID_SOCKET) {
			sprintf_s(tmpstr, "Unable to connect to server!: %ld\n", WSAGetLastError());
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}
		return;
	}
	catch (ArgumentNullException &e0)
	{
		string msg = e0.Message + e0.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentOutOfRangeException &e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException &e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException &e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SecurityException &e4)
	{
		string msg = e4.Message + e4.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException &e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception &e7)
	{
		string msg = e7.Message + e7.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (...)
	{
		clientIsBusy = false;
	}
}

void VMUV_TCP_Cpp::SocketWrapper::ClientContinueRead()
{
	int iResult;

	printf("about to invoke recv()\n");
	Read(ConnectSocket);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
	printf("ClientContinueRead Done\n");
}

/// <summary>
/// Returns all stored trace messages within the <c>TraceLogger</c> object. Call this method after first determining if the 
/// <c>TraceLogger</c> object has any stored messages using the <c>HasTraceMessages</c> method.
/// </summary>
/// <returns>A list of <c>TraceLoggerMessage</c> elements.</returns>
vector<Trace_Logger_Cpp::TraceLoggerMessage> VMUV_TCP_Cpp::SocketWrapper::GetTraceMessages()
{
	return traceLogger.GetAllMessages();
}

/// <summary>
/// Returns true if there are unread messages stored in the <c>TraceLogger</c> object.
/// </summary>
/// <returns>True if unread messages are available. False otherwise.</returns>
bool VMUV_TCP_Cpp::SocketWrapper::HasTraceMessages()
{
	return traceLogger.HasMessages();
}

void VMUV_TCP_Cpp::SocketWrapper::AcceptCB(SOCKET handler)
{
	string methodName = "AcceptCB";

	try
	{
		if (usePing)
			Send(handler, txDataPong);
		else
			Send(handler, txDataPing);
	}
	catch (ArgumentNullException &e0)
	{
		string msg = e0.Message + e0.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentOutOfRangeException &e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException &e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException &e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (NotSupportedException &e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException &e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception &e7)
	{
		string msg = e7.Message + e7.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
}

void VMUV_TCP_Cpp::SocketWrapper::Send(SOCKET handler, vector<char> data)
{
	string methodName = "Send";
	char tmpstr[100];

//	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
//	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
//	WSANETWORKEVENTS NetworkEvents;
//	DWORD EventTotal = 0;

//	HANDLE NewEvent = NULL;

	try
	{
		// Send to server.
		int iResult = send(handler, &data[0], data.size(), 0);
		if (iResult == SOCKET_ERROR) {
			sprintf_s(tmpstr, "send failed with error: %d\n", WSAGetLastError());
			closesocket(handler);
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}
		printf("Bytes Sent: %ld\n", iResult);
		SendCB(handler);
	}
	catch (ArgumentNullException &e0)
	{
		string msg = e0.Message + e0.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentOutOfRangeException &e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException &e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException &e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception &e4)
	{
		string msg = e4.Message + e4.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
}

void VMUV_TCP_Cpp::SocketWrapper::SendCB(SOCKET handler)
{
	string methodName = "SendCB";
	char tmpstr[100];

	try
	{
		// close the socket
		int iResult = closesocket(handler);
		if (iResult == SOCKET_ERROR) {
			sprintf_s(tmpstr, "close failed with error: %d\n", WSAGetLastError());
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}
	}
	catch (SocketException &e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException &e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception &e4)
	{
		string msg = e4.Message + e4.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}

	ResetServer();
}

void VMUV_TCP_Cpp::SocketWrapper::ResetServer()
{
	string methodName = "ResetServer";
	char tmpstr[100];

//	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
//	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
//	DWORD EventTotal = 0;

//	HANDLE NewEvent = NULL;

	try
	{
		//-------------------------
		// Start listening on the socket
		if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
			sprintf_s(tmpstr, "Listen failed with error: %ld\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}

		SOCKET handler = accept(ListenSocket, NULL, NULL);
		if (handler == INVALID_SOCKET) {
			sprintf_s(tmpstr, "accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}
		AcceptCB(handler);
	}
	catch (ArgumentOutOfRangeException &e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException &e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException &e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (NotSupportedException &e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException &e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception &e7)
	{
		string msg = e7.Message + e7.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
}

void VMUV_TCP_Cpp::SocketWrapper::ConnectCB(SOCKET ConnectSocket)
{
	string methodName = "ConnectCB";
	try
	{
		Read(ConnectSocket);
	}
	catch (ArgumentNullException &e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException &e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException &e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentException &e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException &e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception &e7)
	{
		string msg = e7.Message + e7.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (...)
	{
		clientIsBusy = false;
	}
}

void VMUV_TCP_Cpp::SocketWrapper::Read(SOCKET ConnectSocket)
{
	StateObject state;
	string methodName = "Read";
	char tmpstr[100];

//	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
//	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
//	WSANETWORKEVENTS NetworkEvents;
//	DWORD EventTotal = 0;

//	HANDLE NewEvent = NULL;

	try
	{
		state.workSocket = ConnectSocket;
		int iResult = recv(ConnectSocket, &(state.buffer[0]), state.BufferSize, 0);
		if (iResult < 0) {
			sprintf_s(tmpstr, "recv failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			throw SocketException("winsock ", tmpstr);
		}
		else if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			sprintf_s(tmpstr, "Bytes received: %d\n", iResult);
			string msg = tmpstr;
			DebugPrint(msg);

			ReadCB(state);
		}
		else if (iResult == 0) {
			string msg = "Connection closed\n";

			traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
			DebugPrint(msg);
		}
	}
	catch (ArgumentNullException &e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException &e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException &e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentOutOfRangeException &e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception &e7)
	{
		string msg = e7.Message + e7.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (...)
	{
		clientIsBusy = false;
	}
}

void VMUV_TCP_Cpp::SocketWrapper::ReadCB(StateObject state)
{
	string methodName = "ReadCB";
	char tmpstr[100];

	try
	{
		if (state.packetizer.IsPacketValid(state.buffer))
		{
			if (usePing)
			{
				rxDataPing = state.packetizer.UnpackData(state.buffer);
				rxTypePing = state.packetizer.GetPacketType(state.buffer);
				usePing = false;
			}
			else
			{
				rxDataPong = state.packetizer.UnpackData(state.buffer);
				rxTypePong = state.packetizer.GetPacketType(state.buffer);
				usePing = true;
			}

			numPacketsRead++;
			sprintf_s(tmpstr, "numPacketsRead: %d\n", numPacketsRead);
			string msg = tmpstr;
			DebugPrint(msg);
		}
	}
	catch (ArgumentNullException &e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException &e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException &e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentException &e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException &e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception &e7)
	{
		string msg = e7.Message + e7.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}

	clientIsBusy = false;
}

void VMUV_TCP_Cpp::SocketWrapper::DebugPrint(string s)
{
#if DEBUG
	Console.WriteLine(s);
#endif
}

//--------------------------------------------------------------------
// code for testing server and client using separate threads

DWORD  VMUV_TCP_Cpp::SocketWrapper::WASAPIServerTestThread(LPVOID Context)
{
	VMUV_TCP_Cpp::SocketWrapper *pSocketWrapper = static_cast<VMUV_TCP_Cpp::SocketWrapper *>(Context);

	try {
		pSocketWrapper->runServerTest();
	}
	catch (...) {
		int i = 0; // place to hang a break point
	}
	return 0;
}

void VMUV_TCP_Cpp::SocketWrapper::StartServerTestThread()
{
	m_ServerTestThread = CreateThread(NULL, 0, WASAPIServerTestThread, this, 0, NULL);
}

void VMUV_TCP_Cpp::SocketWrapper::runServerTest()
{
	serverTestRunning = true;
	StartServer();
	ContinueServer();
	serverTestRunning = false;
}

void VMUV_TCP_Cpp::SocketWrapper::StopServerTestThread()
{
	if (serverTestRunning) {
		int ret = TerminateThread(m_ServerTestThread, 0);
		serverTestRunning = false;
	}
}


DWORD  VMUV_TCP_Cpp::SocketWrapper::WASAPIClientTestThread(LPVOID Context)
{
	VMUV_TCP_Cpp::SocketWrapper *pSocketWrapper = static_cast<VMUV_TCP_Cpp::SocketWrapper *>(Context);

	try {
		pSocketWrapper->runClientTest();
	}
	catch (...) {
		int i = 0; // place to hang a break point
	}
	return 0;
}

void VMUV_TCP_Cpp::SocketWrapper::StartClientTestThread()
{
	m_ClientTestThread = CreateThread(NULL, 0, WASAPIClientTestThread, this, 0, NULL);
}

void VMUV_TCP_Cpp::SocketWrapper::runClientTest()
{
	clientTestRunning = true;
	ClientStartRead();
	clientTestRunning = false;
}

void VMUV_TCP_Cpp::SocketWrapper::StopClientTestThread()
{
	if (clientTestRunning) {
		int ret = TerminateThread(m_ClientTestThread, 0);
		clientTestRunning = false;
	}
}
