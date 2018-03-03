// Packetizer.h

#pragma once
#include <vector>
#include <string>

using namespace std;

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

		const char sync1 = 0x69;
		const char sync2 = 0xee;
		const int numOverHeadBytes = 7;
		const char sycn1Loc = 0x00;
		const char sycn2Loc = 0x01;
		const char typeLoc = 0x02;
		const char lenMSBLoc = 0x03;
		const char lenLSBLoc = 0x04;
		const char dataStartLoc = 0x05;

	protected:
		void BuildHeader(vector<char> &packet, char type, short len);
		short CalculateCheckSumFromPayload(vector<char> payload, short len);

	public:
		vector<char> PacketizeData(vector<char> payload, char type) throw(ArgumentOutOfRangeException);
		bool IsPacketValid(vector<char> packet);
		char GetPacketType(vector<char> packet)  throw(ArgumentException);
		vector<char> UnpackData(vector<char> packet) throw(ArgumentException);
	};

}
