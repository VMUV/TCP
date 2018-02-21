// StateObject.cpp

#include "stdafx.h"

#include "StateObject.h"


VMUV_TCP_Cpp::StateObject::StateObject()
{
	workSocket = INVALID_SOCKET;
	buffer.resize(BufferSize);
}


VMUV_TCP_Cpp::StateObject::~StateObject()
{
}

