// Packetizer.h

#pragma once
#include <vector>
#include <string>

using namespace std;

typedef unsigned char byte;
#define Int16MaxValue 32767
#define ShortMaxValue 32767


namespace VMUV_TCP_Cpp
{

	class ArgumentOutOfRangeException
	{
	public:
		ArgumentOutOfRangeException(string arg1, string arg2)
		{
			Message = arg1;
			StackTrace = arg2;
		};

		string Message;
		string StackTrace;
	};
	class ArgumentException
	{
	public:
		ArgumentException(string arg1, string arg2)
		{
			Message = arg1;
			StackTrace = arg2;
		};

		string Message;
		string StackTrace;
	};
	class OverflowException
	{
	public:
		OverflowException() {};
	};
	class IndexOutOfRangeException
	{
	public:
		IndexOutOfRangeException() {};
	};
	class NullReferenceException
	{
	public:
		NullReferenceException() {};
	};
	class Exception
	{
	public:
		Exception(string arg1, string arg2)
		{
			Message = arg1;
			StackTrace = arg2;
		};

		string Message;
		string StackTrace;
	};

	//-------------------------------------------------------------------
	class Packetizer
	{
	public:
		Packetizer();
		virtual ~Packetizer();

		const byte sync1 = 0x69;
		const byte sync2 = 0xee;
		const int numOverHeadBytes = 7;
		const byte sycn1Loc = 0x00;
		const byte sycn2Loc = 0x01;
		const byte typeLoc = 0x02;
		const byte lenMSBLoc = 0x03;
		const byte lenLSBLoc = 0x04;
		const byte dataStartLoc = 0x05;

	protected:
		void BuildHeader(vector<byte> &packet, byte type, short len);
		short CalculateCheckSumFromPayload(vector<byte> payload, short len);

	public:
		vector<byte> PacketizeData(vector<byte> payload, byte type);
		bool IsPacketValid(vector<byte> packet);
		byte GetPacketType(vector<byte> packet);
		vector<byte> UnpackData(vector<byte> packet);
	};

}
