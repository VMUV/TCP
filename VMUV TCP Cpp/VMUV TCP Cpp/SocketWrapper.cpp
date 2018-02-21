// SocketWrapper.cpp

#include "stdafx.h"

#include "StateObject.h"
#include "SocketWrapper.h"

WSADATA wsaData;

VMUV_TCP_Cpp::SocketWrapper::SocketWrapper()
{
	listener = INVALID_SOCKET;
	txDataPing = { 0 };  // Do this incase Start() is called before the user sets any data
	txDataPong = { 0 };  // Do this incase Start() is called before the user sets any data
	rxDataPing = { 0 };  // Do this incase GetRxData() is called before the user gets any data
	rxDataPong = { 0 };  // Do this incase GetRxData() is called before the user gets any data
	usePing = true;
	clientIsBusy = false;
	moduleName = "SocketWrapper.cs";
	numPacketsRead = 0;
}


		/// <summary>
		/// Instantiates a new instance of <c>SocketWrapper</c> configured as either a client or a server.
		/// </summary>
		/// <param name="configuration"></param>
VMUV_TCP_Cpp::SocketWrapper::SocketWrapper(Configuration configuration)
{
	config = configuration;
	listener = INVALID_SOCKET;
	txDataPing = { 0 };  // Do this incase Start() is called before the user sets any data
	txDataPong = { 0 };  // Do this incase Start() is called before the user sets any data
	rxDataPing = { 0 };  // Do this incase GetRxData() is called before the user gets any data
	rxDataPong = { 0 };  // Do this incase GetRxData() is called before the user gets any data
	usePing = true;
	clientIsBusy = false;
	moduleName = "SocketWrapper.cs";
	numPacketsRead = 0;
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
	catch (ArgumentOutOfRangeException e0)
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

	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS NetworkEvents;
	DWORD EventTotal = 0;

	HANDLE NewEvent = NULL;

	if (config != Configuration::server)
		return;

	try
	{
		// the following from document msdn.microsoft.com ...> Using Winsock > Getting Started With Winsock
		// initializa Winsock
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			throw new SocketException("winsock ", "unable to start winsock");
		}

		sprintf_s(portstr, "%d", port);

		struct addrinfo *result = NULL, *ptr = NULL, hints;

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
			throw new SocketException("winsock ", tmpstr);
		}
		
		string msg = "TCP Server successfully started on port " + string(portstr);

		//-------------------------
		// Create a listening socket
		listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

		if (listener == INVALID_SOCKET) {
			sprintf_s(tmpstr, "Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Bind the listening socket
		// Setup the TCP listening socket
		iResult = bind(listener, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			sprintf_s(tmpstr, "bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}
		freeaddrinfo(result);

		//-------------------------
		// Start listening on the socket
		if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
			sprintf_s(tmpstr, "Listen failed with error: %ld\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		// Accept a handler socket
		SOCKET handler = accept(listener, NULL, NULL);
		if (handler == INVALID_SOCKET) {
			sprintf_s(tmpstr, "accept failed: %d\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		AcceptCB(handler);

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
	}
	catch (ArgumentNullException e0)
	{
		string msg = e0.Message + e0.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentOutOfRangeException e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SecurityException e4)
	{
		string msg = e4.Message + e4.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (NotSupportedException e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception e7)
	{
		string msg = e7.Message + e7.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
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

	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS NetworkEvents;
	DWORD EventTotal = 0;

	HANDLE NewEvent = NULL;

	if (clientIsBusy || (config != Configuration::client))
		return;

	try
	{
		sprintf_s(portstr, "%d", port);

		struct addrinfo *result = NULL, *ptr = NULL, hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		int iResult = getaddrinfo(NULL, portstr, &hints, &result);
		if (iResult != 0) {
			sprintf_s(tmpstr, "getaddrinfo failed: %d\n", iResult);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		clientIsBusy = true;
		// Attempt to connect to the first address returned by
		// the call to getaddrinfo
		ptr = result;

		// Create a SOCKET for connecting to server
		SOCKET client = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (client == INVALID_SOCKET) {
			sprintf_s(tmpstr, "Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		freeaddrinfo(result);

		// Connect to server.
		iResult = connect(client, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(client);
			client = INVALID_SOCKET;
		}
		else {
			ConnectCB(client);
		}

		// Should really try the next address returned by getaddrinfo
		// if the connect call failed
		// But for this simple example we just free the resources
		// returned by getaddrinfo and print an error message

		if (client == INVALID_SOCKET) {
			sprintf_s(tmpstr, "Unable to connect to server!\n");
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}
		return;
	}
	catch (ArgumentNullException e0)
	{
		string msg = e0.Message + e0.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentOutOfRangeException e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SecurityException e4)
	{
		string msg = e4.Message + e4.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception e7)
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
	catch (ArgumentNullException e0)
	{
		string msg = e0.Message + e0.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentOutOfRangeException e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (NotSupportedException e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception e7)
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

	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS NetworkEvents;
	DWORD EventTotal = 0;

	HANDLE NewEvent = NULL;

	try
	{
		// Send to server.
		int iResult = send(handler, &data[0], (int)strlen(&data[0]), 0);
		if (iResult == SOCKET_ERROR) {
			sprintf_s(tmpstr, "send failed with error: %d\n", WSAGetLastError());
			closesocket(handler);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}
		SendCB(handler);
	}
	catch (ArgumentNullException e0)
	{
		string msg = e0.Message + e0.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentOutOfRangeException e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception e4)
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
			throw new SocketException("winsock ", tmpstr);
		}
	}
	catch (SocketException e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception e4)
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

	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	DWORD EventTotal = 0;

	HANDLE NewEvent = NULL;

	try
	{
		//-------------------------
		// Start listening on the socket
		if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
			sprintf_s(tmpstr, "Listen failed with error: %ld\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		SOCKET handler = accept(listener, NULL, NULL);
		if (handler == INVALID_SOCKET) {
			sprintf_s(tmpstr, "accept failed: %d\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}
		AcceptCB(handler);
	}
	catch (ArgumentOutOfRangeException e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (NotSupportedException e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception e7)
	{
		string msg = e7.Message + e7.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
}

void VMUV_TCP_Cpp::SocketWrapper::ConnectCB(SOCKET client)
{
	string methodName = "ConnectCB";
	try
	{
		Read(client);
	}
	catch (ArgumentNullException e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentException e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception e7)
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

void VMUV_TCP_Cpp::SocketWrapper::Read(SOCKET client)
{
	StateObject state;
	string methodName = "Read";
	char tmpstr[100];

	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS NetworkEvents;
	DWORD EventTotal = 0;

	HANDLE NewEvent = NULL;

	try
	{
		state.workSocket = client;
		int iResult = recv(client, &(state.buffer[0]), state.BufferSize, 0);
		if (iResult < 0) {
			sprintf_s(tmpstr, "recv failed with error: %d\n", WSAGetLastError());
			closesocket(client);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}
		else if (iResult > 0) {
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
	catch (ArgumentNullException e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentOutOfRangeException e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception e7)
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
	catch (ArgumentNullException e1)
	{
		string msg = e1.Message + e1.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (SocketException e2)
	{
		string msg = e2.Message + e2.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ObjectDisposedException e3)
	{
		string msg = e3.Message + e3.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (ArgumentException e5)
	{
		string msg = e5.Message + e5.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (InvalidOperationException e6)
	{
		string msg = e6.Message + e6.StackTrace;

		traceLogger.QueueMessage(traceLogger.BuildMessage(moduleName, methodName, msg));
		DebugPrint(msg);
	}
	catch (Exception e7)
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
