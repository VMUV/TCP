// SocketWrapper.cpp

/////////////////////////////////////////////////////////////////////////////////
// this is an early, incomplete checkin, in process of porting C# code to C++.
//
// original C# code is generaly commented out, indicated by //C# at the start of the line.
//
// there are 2 versions of the ported code present:
// code using non-blocking multiple event methods is enclosed in #ifdef ASYNC_SOCKETS ... #else // not ASYNC_SOCKETS blocks
// code using simple blocking calls is enclosed within #else // not ASYNC_SOCKETS ... #endif // ASYNC_SOCKETS blocks
// I'm hoping we can get by with the later - but retained the former, having allready written it.
//

#include "stdafx.h"

#include "SocketWrapper.h"

//using System;
//using System.Net;
//using System.Net.Sockets;
//using System.Security;
//using Trace_Logger_CSharp;

WSADATA wsaData;

VMUV_TCP_Cpp::SocketWrapper::SocketWrapper()
{
//	packetizer = new Packetizer();
//	traceLogger = new Trace_Logger_Cpp::TraceLogger();
//	listener = null;
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
//	packetizer = new Packetizer();
//	traceLogger = new TraceLogger();
//	listener = null;
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
//	if (packetizer) delete packetizer;
//	if (traceLogger) delete traceLogger;
}

/// <summary>
/// Sets the data from <c>payload</c> into the transmit data buffer.
/// </summary>
/// <param name="payload"></param>
/// <param name="type"></param>
void VMUV_TCP_Cpp::SocketWrapper::ServerSetTxData(vector<byte> payload, byte type)
{
	string methodName = "ServerSetTxData";
	try
	{
		if (usePing)
		{
			txDataPing = packetizer.PacketizeData(payload, (byte)type);
			usePing = false;
		}
		else
		{
			txDataPong = packetizer.PacketizeData(payload, (byte)type);
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
/// <returns>byte buffer with a copy of the most recently receieved valid data payload.</returns>
vector<byte> VMUV_TCP_Cpp::SocketWrapper::ClientGetRxData()
{
	if (usePing)
		return rxDataPong;
	else
		return rxDataPing;
}

/// <summary>
/// Acquires the most recently received payload type.
/// </summary>
/// <returns>byte with the most recent payload type. </returns>
byte VMUV_TCP_Cpp::SocketWrapper::ClientGetRxType()
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
//C#		IPEndPoint localEP = new IPEndPoint(IPAddress.Loopback, port);

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
//C#		listener = new SOCKET(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
		listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

		if (listener == INVALID_SOCKET) {
			sprintf_s(tmpstr, "Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Bind the listening socket
//C#		listener.Bind(localEP);
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

#ifdef ASYNC_SOCKETS
		//-------------------------
		// Create a new event
		NewEvent = WSACreateEvent();
		if (NewEvent == NULL) {
			sprintf_s(tmpstr, "WSACreateEvent failed with error: %d\n", GetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Associate event types FD_ACCEPT
		// with the listening socket and NewEvent
		iResult = WSAEventSelect(listener, NewEvent, FD_ACCEPT );
		if (iResult != 0) {
			sprintf_s(tmpstr, "WSAEventSelect failed with error: %d\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}
		
		//-------------------------
		// Start listening on the socket
//C#		listener.Listen(100);
		if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
			sprintf_s(tmpstr, "Listen failed with error: %ld\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

//C#		listener.BeginAccept(new AsyncCallback(AcceptCB), listener);
		//-------------------------
		// Add the socket and event to the arrays, increment number of events
		SocketArray[EventTotal] = listener;
		EventArray[EventTotal] = NewEvent;
		EventTotal++;

		//-------------------------
		// Wait for network events on all sockets
		DWORD Index;
		Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);
		Index = Index - WSA_WAIT_EVENT_0;

		//-------------------------
		// Iterate through all events and enumerate
		// if the wait does not fail.
		DWORD i;
		for (i = Index; i < EventTotal; i++) {
			Index = WSAWaitForMultipleEvents(1, &EventArray[i], TRUE, 1000, FALSE);
			if ((Index != WSA_WAIT_FAILED) && (Index != WSA_WAIT_TIMEOUT)) {
				iResult = WSAEnumNetworkEvents(SocketArray[i], EventArray[i], &NetworkEvents);
				if (iResult == SOCKET_ERROR) {
					sprintf_s(tmpstr, "WSAEnumNetworkEvents failed with error: %ld\n", WSAGetLastError());
					closesocket(listener);
					WSACleanup();
					throw new SocketException("winsock ", tmpstr);
				}

				if (NetworkEvents.lNetworkEvents & FD_ACCEPT) {
					AcceptCB(listener);
				}
			}
		}

#else // not ASYNC_SOCKETS

		//-------------------------
		// Start listening on the socket
//C#		listener.Listen(100);
		if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
			sprintf_s(tmpstr, "Listen failed with error: %ld\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		// Accept a client socket
//C#		listener.BeginAccept(new AsyncCallback(AcceptCB), listener);
		SOCKET client = accept(listener, NULL, NULL);
		if (client == INVALID_SOCKET) {
			sprintf_s(tmpstr, "accept failed: %d\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		AcceptCB(listener);
#endif // ASYNC_SOCKETS

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
//C#		IPEndPoint remoteEP = new IPEndPoint(IPAddress.Loopback, port);

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
//C#		SOCKET client = new SOCKET(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
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

#ifdef ASYNC_SOCKETS
		//-------------------------
		// Create a new event
		NewEvent = WSACreateEvent();
		if (NewEvent == NULL) {
			sprintf_s(tmpstr, "WSACreateEvent failed with error: %d\n", GetLastError());
			closesocket(client);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

//C#		client.BeginConnect(remoteEP, new AsyncCallback(ConnectCB), client);
		//-------------------------
		// Associate event types FD_CONNECT
		// with the listening socket and NewEvent
		iResult = WSAEventSelect(client, NewEvent, FD_CONNECT);
		if (iResult != 0) {
			sprintf_s(tmpstr, "WSAEventSelect failed with error: %d\n", WSAGetLastError());
			closesocket(client);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Add the socket and event to the arrays, increment number of events
		SocketArray[EventTotal] = client;
		EventArray[EventTotal] = NewEvent;
		EventTotal++;

		//-------------------------
		// Wait for network events on all sockets
		DWORD Index;
		Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);
		Index = Index - WSA_WAIT_EVENT_0;

		//-------------------------
		// Iterate through all events and enumerate
		// if the wait does not fail.
		DWORD i;
		for (i = Index; i < EventTotal; i++) {
			Index = WSAWaitForMultipleEvents(1, &EventArray[i], TRUE, 1000, FALSE);
			if ((Index != WSA_WAIT_FAILED) && (Index != WSA_WAIT_TIMEOUT)) {
				iResult = WSAEnumNetworkEvents(SocketArray[i], EventArray[i], &NetworkEvents);
				if (iResult == SOCKET_ERROR) {
					sprintf_s(tmpstr, "WSAEnumNetworkEvents failed with error: %ld\n", WSAGetLastError());
					closesocket(client);
					WSACleanup();
					throw new SocketException("winsock ", tmpstr);
				}

				if (NetworkEvents.lNetworkEvents & FD_CONNECT) {
					ConnectCB(client);
				}
			}
		}

#else // not ASYNC_SOCKETS
//C#		client.BeginConnect(remoteEP, new AsyncCallback(ConnectCB), client);
		// Connect to server.
		iResult = connect(client, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(client);
			client = INVALID_SOCKET;
		}
		ConnectCB(client);
#endif // ASYNC_SOCKETS

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
//C#	finally
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

//C#	void VMUV_TCP_Cpp::SocketWrapper::AcceptCB(IAsyncResult ar)
void VMUV_TCP_Cpp::SocketWrapper::AcceptCB(SOCKET handler)
{
	string methodName = "AcceptCB";

	try
	{
//C#		SOCKET local = (SOCKET)ar.AsyncState;
//C#		SOCKET handler = listener.EndAccept(ar);

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

void VMUV_TCP_Cpp::SocketWrapper::Send(SOCKET handler, vector<byte> data)
{
	string methodName = "Send";
	char tmpstr[100];
	char *sendbuf = "Client: sending data test";

	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS NetworkEvents;
	DWORD EventTotal = 0;

	HANDLE NewEvent = NULL;

	try
	{
//C#		handler.BeginSend(data, 0, data.size(), 0, new AsyncCallback(SendCB), handler);

#ifdef ASYNC_SOCKETS
		//-------------------------
		// Create a new event
		NewEvent = WSACreateEvent();
		if (NewEvent == NULL) {
			sprintf_s(tmpstr, "WSACreateEvent failed with error: %d\n", GetLastError());
			closesocket(handler);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Associate event types FD_WRITE
		// with the listening socket and NewEvent
		int iResult = WSAEventSelect(handler, NewEvent, FD_WRITE);
		if (iResult != 0) {
			sprintf_s(tmpstr, "WSAEventSelect failed with error: %d\n", WSAGetLastError());
			closesocket(handler);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Add the socket and event to the arrays, increment number of events
		SocketArray[EventTotal] = handler;
		EventArray[EventTotal] = NewEvent;
		EventTotal++;

		//-------------------------
		// Wait for network events on all sockets
		DWORD Index;
		Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);
		Index = Index - WSA_WAIT_EVENT_0;

		//-------------------------
		// Iterate through all events and enumerate
		// if the wait does not fail.
		DWORD i;
		for (i = Index; i < EventTotal; i++) {
			Index = WSAWaitForMultipleEvents(1, &EventArray[i], TRUE, 1000, FALSE);
			if ((Index != WSA_WAIT_FAILED) && (Index != WSA_WAIT_TIMEOUT)) {
				iResult = WSAEnumNetworkEvents(SocketArray[i], EventArray[i], &NetworkEvents);
				if (iResult == SOCKET_ERROR) {
					sprintf_s(tmpstr, "WSAEnumNetworkEvents failed with error: %ld\n", WSAGetLastError());
					closesocket(handler);
					WSACleanup();
					throw new SocketException("winsock ", tmpstr);
				}

				if (NetworkEvents.lNetworkEvents & FD_WRITE) {
					SendCB(handler);
				}
			}
		}

#else // not ASYNC_SOCKETS
		// Send to server.
		int iResult = send(handler, sendbuf, (int)strlen(sendbuf), 0);
		if (iResult == SOCKET_ERROR) {
			sprintf_s(tmpstr, "send failed with error: %d\n", WSAGetLastError());
			closesocket(handler);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}
		SendCB(handler);
#endif // ASYNC_SOCKETS

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

//C#	void VMUV_TCP_Cpp::SocketWrapper::SendCB(IAsyncResult ar)
void VMUV_TCP_Cpp::SocketWrapper::SendCB(SOCKET handler)
{
	string methodName = "SendCB";
	char tmpstr[100];

	try
	{
//C#		SOCKET handler = (SOCKET)ar.AsyncState;
//C#		int numBytesSent = handler.EndSend(ar);

//C#		handler.Shutdown(SocketShutdown.Both);
//C#		handler.Close();
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
//C#		listener.Listen(100);
//C#		listener.BeginAccept(new AsyncCallback(AcceptCB), listener);

#ifdef ASYNC_SOCKETS
		//-------------------------
		// Create a new event
		NewEvent = WSACreateEvent();
		if (NewEvent == NULL) {
			sprintf_s(tmpstr, "WSACreateEvent failed with error: %d\n", GetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Associate event types FD_ACCEPT
		// with the listening socket and NewEvent
		int iResult = WSAEventSelect(listener, NewEvent, FD_ACCEPT);
		if (iResult != 0) {
			sprintf_s(tmpstr, "WSAEventSelect failed with error: %d\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Start listening on the socket
//C#		listener.Listen(100);
		if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
			sprintf_s(tmpstr, "Listen failed with error: %ld\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Add the socket and event to the arrays, increment number of events
		SocketArray[EventTotal] = listener;
		EventArray[EventTotal] = NewEvent;
		EventTotal++;

		//-------------------------
		// Wait for network events on all sockets
		DWORD Index;
		Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);
		Index = Index - WSA_WAIT_EVENT_0;

		//-------------------------
		// Iterate through all events and enumerate
		// if the wait does not fail.
		DWORD i;
		for (i = Index; i < EventTotal; i++) {
			Index = WSAWaitForMultipleEvents(1, &EventArray[i], TRUE, 1000, FALSE);
			if ((Index != WSA_WAIT_FAILED) && (Index != WSA_WAIT_TIMEOUT)) {
				iResult = WSAEnumNetworkEvents(SocketArray[i], EventArray[i], &NetworkEvents);
				if (iResult == SOCKET_ERROR) {
					sprintf_s(tmpstr, "WSAEnumNetworkEvents failed with error: %ld\n", WSAGetLastError());
					closesocket(listener);
					WSACleanup();
					throw new SocketException("winsock ", tmpstr);
				}

				if (NetworkEvents.lNetworkEvents & FD_ACCEPT) {
					AcceptCB(listener);
				}
			}
		}

#else // not ASYNC_SOCKETS
		//-------------------------
		// Start listening on the socket
//C#		listener.Listen(100);
		if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
			sprintf_s(tmpstr, "Listen failed with error: %ld\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		SOCKET client = accept(listener, NULL, NULL);
		if (client == INVALID_SOCKET) {
			sprintf_s(tmpstr, "accept failed: %d\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		AcceptCB(listener);
#endif // ASYNC_SOCKETS

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

//C#	void VMUV_TCP_Cpp::SocketWrapper::ConnectCB(IAsyncResult ar)
void VMUV_TCP_Cpp::SocketWrapper::ConnectCB(SOCKET client)
{
	string methodName = "ConnectCB";
	try
	{
//C#		SOCKET client = (SOCKET)ar.AsyncState;

//C#		client.EndConnect(ar);
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
//	finally
	catch (...)
	{
		clientIsBusy = false;
	}
}

void VMUV_TCP_Cpp::SocketWrapper::Read(SOCKET client)
{
	StateObject state = new StateObject();
	string methodName = "Read";
	char tmpstr[100];

#define DEFAULT_BUFLEN 512
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS NetworkEvents;
	DWORD EventTotal = 0;

	HANDLE NewEvent = NULL;

	try
	{
		state.workSocket = client;
//C#		client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0, new AsyncCallback(ReadCB), state);

#ifdef ASYNC_SOCKETS
		//-------------------------
		// Create a new event
		NewEvent = WSACreateEvent();
		if (NewEvent == NULL) {
			sprintf_s(tmpstr, "WSACreateEvent failed with error: %d\n", GetLastError());
			closesocket(client);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Associate event types FD_READ
		// with the listening socket and NewEvent
		int iResult = WSAEventSelect(client, NewEvent, FD_READ);
		if (iResult != 0) {
			sprintf_s(tmpstr, "WSAEventSelect failed with error: %d\n", WSAGetLastError());
			closesocket(client);
			WSACleanup();
			throw new SocketException("winsock ", tmpstr);
		}

		//-------------------------
		// Add the socket and event to the arrays, increment number of events
		SocketArray[EventTotal] = client;
		EventArray[EventTotal] = NewEvent;
		EventTotal++;

		//-------------------------
		// Wait for network events on all sockets
		DWORD Index;
		Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);
		Index = Index - WSA_WAIT_EVENT_0;

		//-------------------------
		// Iterate through all events and enumerate
		// if the wait does not fail.
		DWORD i;
		for (i = Index; i < EventTotal; i++) {
			Index = WSAWaitForMultipleEvents(1, &EventArray[i], TRUE, 1000, FALSE);
			if ((Index != WSA_WAIT_FAILED) && (Index != WSA_WAIT_TIMEOUT)) {
				iResult = WSAEnumNetworkEvents(SocketArray[i], EventArray[i], &NetworkEvents);
				if (iResult == SOCKET_ERROR) {
					sprintf_s(tmpstr, "WSAEnumNetworkEvents failed with error: %ld\n", WSAGetLastError());
					closesocket(client);
					WSACleanup();
					throw new SocketException("winsock ", tmpstr);
				}

				if (NetworkEvents.lNetworkEvents & FD_READ) {
					ReadCB();
				}
			}
		}

#else // not ASYNC_SOCKETS
		int iResult = recv(client, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());

		// what about ReadCB?
#endif // ASYNC_SOCKETS

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
//	finally
	catch (...)
	{
		clientIsBusy = false;
	}
}

//C#	void VMUV_TCP_Cpp::SocketWrapper::ReadCB(IAsyncResult ar)
void VMUV_TCP_Cpp::SocketWrapper::ReadCB(SOCKET client)
{
	string methodName = "ReadCB";

	try
	{
//C#		StateObject state = (StateObject)ar.AsyncState;
//C#		SOCKET client = state.workSocket;
//C#		int numBytesRead = client.EndReceive(ar);

//C#		if (numBytesRead > 0)
//C#		{
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
				DebugPrint(numPacketsRead.ToString());
			}
//C#		}
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
