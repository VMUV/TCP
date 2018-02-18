// StateObject.h

#pragma once
#include <vector>
//#include <Afxsock.h>
#include <winsock.h>

using namespace std;

#define byte unsigned char

#include "Packetizer.h"

//using System.Net.Sockets;

namespace VMUV_TCP_Cpp
{
	class StateObject
	{
	public:
		StateObject();
		virtual ~StateObject();

		SOCKET workSocket;
		const int BufferSize = 1024;
		vector<byte> buffer;
		Packetizer packetizer;
	};
}
