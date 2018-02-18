// StateObject.cpp

#include "stdafx.h"

#include "StateObject.h"

//using System.Net.Sockets;

VMUV_TCP_Cpp::StateObject::StateObject()
{
		workSocket = null;
//		buffer = new byte[BufferSize];
		vector<byte> n(BufferSize);
		buffer = n;
//		packetizer = new Packetizer();
}


VMUV_TCP_Cpp::StateObject::~StateObject()
{
//	if (buffer) delete buffer;
//	if (packetizer) delete packetizer;
}

