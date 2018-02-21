// StateObject.h

#pragma once
#include <vector>
#include <winsock2.h>

using namespace std;


#include "Packetizer.h"

#define STATE_BUFFER_SIZE 1024

namespace VMUV_TCP_Cpp
{
	class StateObject
	{
	public:
		StateObject();
		virtual ~StateObject();

		SOCKET workSocket;
		const int BufferSize = STATE_BUFFER_SIZE;
		vector<char> buffer;
		Packetizer packetizer;
	};
}
