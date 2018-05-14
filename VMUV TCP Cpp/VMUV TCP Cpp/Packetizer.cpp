// Packetizer.cpp

#include "stdafx.h"

#include "Packetizer.h"


VMUV_TCP_Cpp::Packetizer::Packetizer()
{
}


VMUV_TCP_Cpp::Packetizer::~Packetizer()
{
}


void VMUV_TCP_Cpp::Packetizer::BuildHeader(vector<byte> &packet, byte type, short len)
{
	packet[sycn1Loc] = sync1;
	packet[sycn2Loc] = sync2;
	packet[typeLoc] = type;
	packet[lenMSBLoc] = (byte)((len >> 8) & 0xff);
	packet[lenLSBLoc] = (byte)(len & 0xff);
}

short VMUV_TCP_Cpp::Packetizer::CalculateCheckSumFromPayload(vector<byte> payload, short len)
{
	short chkSum = 0;

	if (payload.size() >= len)
	{
		for (short i = 0; i < len; i++)
			chkSum += (short)(payload[i] & 0xff);
	}

	return chkSum;
}

vector<byte> VMUV_TCP_Cpp::Packetizer::PacketizeData(vector<byte> payload, byte type)  throw(ArgumentOutOfRangeException)
{
	short len = 0;

	try
	{
		if (payload.size() > Int16MaxValue) {
			throw ArgumentOutOfRangeException("payload",
				"Length is greater than short.MaxValue");
		}
		else
			len = (short)(payload.size() & 0xffff);
	}
	catch (OverflowException &e)
	{
		// TODO:
	}

	vector<byte> packet(numOverHeadBytes + len);
	short chkSum = CalculateCheckSumFromPayload(payload, len);

	BuildHeader(packet, type, len);

	for (short i = 0; i < len; i++)
		packet[5 + i] = payload[i];

	packet[5 + len] = (byte)((chkSum >> 8) & 0xff);
	packet[6 + len] = (byte)(chkSum & 0xff);

	return packet;
}

bool VMUV_TCP_Cpp::Packetizer::IsPacketValid(vector<byte> packet)
{
	try
	{
		// bd: I added this to avoid exceptions from accessing header bytes
		if (packet.size() < numOverHeadBytes)
			return false;

		if (packet[sycn1Loc] != sync1)
			return false;

		if (packet[sycn2Loc] != sync2)
			return false;

		short len = (short)(packet[lenMSBLoc]);
		len <<= 8;
		len |= (short)(packet[lenLSBLoc] & 0xff);

		if (len > (packet.size() - numOverHeadBytes))
			return false;

		vector<byte> payload(len);

		for (short i = 0; i < len; i++)
			payload[i] = packet[5 + i];

		short calcChkSum = CalculateCheckSumFromPayload(payload, len);
		short recChkSum = (short)packet[len + 5];
		recChkSum <<= 8;
		recChkSum |= (short)(packet[len + 6] & 0xff);

		if (recChkSum != calcChkSum)
			return false;
	}
	catch (IndexOutOfRangeException &e0)
	{
		// TODO:
		return false;
	}
	catch (NullReferenceException &e1)
	{
		// TODO:
		return false;
	}
	catch (OverflowException &e2)
	{
		// TODO:
		return false;
	}
	catch (Exception &e3)
	{
		// TODO:
		return false;
	}

	return true;
}

byte VMUV_TCP_Cpp::Packetizer::GetPacketType(vector<byte> packet) throw(ArgumentException)
{
	if (IsPacketValid(packet))
		return packet[typeLoc];
	else
		throw ArgumentException("packet", "type is not valid");
}

vector<byte> VMUV_TCP_Cpp::Packetizer::UnpackData(vector<byte> packet) throw(ArgumentException)
{
	if (IsPacketValid(packet))
	{
		short len = (short)(packet[lenMSBLoc] & 0xff);
		len <<= 8;
		len |= (short)(packet[lenLSBLoc] & 0xff);

		vector<byte> rtn(len);

		for (short i = 0; i < len; i++)
			rtn[i] = packet[dataStartLoc + i];

		return rtn;
	}
	else
		throw ArgumentException("packet", "packet is not valid");
}
