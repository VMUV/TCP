using System;

namespace VMUV_TCP_CSharp
{
    public class Packetizer
    {
        public const byte sync1 = 0x69;
        public const byte sync2 = 0xee;
        public const int numOverHeadBytes = 7;
        public const byte sycn1Loc = 0x00;
        public const byte sycn2Loc = 0x01;
        public const byte typeLoc = 0x02;
        public const byte lenMSBLoc = 0x03;
        public const byte lenLSBLoc = 0x04;
        public const byte dataStartLoc = 0x05;

        protected void BuildHeader(byte[] packet, byte type, short len)
        {
            packet[sycn1Loc] = sync1;
            packet[sycn2Loc] = sync2;
            packet[typeLoc] = type;
            packet[lenMSBLoc] = (byte)((len >> 8) & 0xff);
            packet[lenLSBLoc] = (byte)(len & 0xff);
        }

        protected short CalculateCheckSumFromPayload(byte[] payload, short len)
        {
            short chkSum = 0;

            if (payload != null)
            {
                for (short i = 0; i < len; i++)
                    chkSum += (short)(payload[i] & 0xff);
            }

            return chkSum;
        }

        public byte[] PacketizeData(byte[] payload, byte type)
        {
            short len = 0;

            if (payload != null)
            {
                try
                {
                    if (payload.Length > short.MaxValue)
                        throw new ArgumentOutOfRangeException("payload",
                            "Length is greater than short.MaxValue");
                    else
                        len = (short)(payload.Length & 0xffff);
                }
                catch (OverflowException e)
                {
                    // TODO:
                }
            }

            byte[] packet = new byte[numOverHeadBytes + len];
            short chkSum = CalculateCheckSumFromPayload(payload, len);

            BuildHeader(packet, type, len);

            for (short i = 0; i < len; i++)
                packet[5 + i] = payload[i];

            packet[5 + len] = (byte)((chkSum >> 8) & 0xff);
            packet[6 + len] = (byte)(chkSum & 0xff);

            return packet;
        }

        public bool IsPacketValid(byte[] packet)
        {
            try
            {
                if (packet[sycn1Loc] != sync1)
                    return false;

                if (packet[sycn2Loc] != sync2)
                    return false;

                short len = (short)(packet[lenMSBLoc]);
                len <<= 8;
                len |= (short)(packet[lenLSBLoc] & 0xff);

                if (len > (packet.Length - numOverHeadBytes))
                    return false;

                byte[] payload = new byte[len];

                for (short i = 0; i < len; i++)
                    payload[i] = packet[5 + i];

                short calcChkSum = CalculateCheckSumFromPayload(payload, len);
                short recChkSum = (short)packet[len + 5];
                recChkSum <<= 8;
                recChkSum |= (short)(packet[len + 6] & 0xff);

                if (recChkSum != calcChkSum)
                    return false;
            }
            catch (IndexOutOfRangeException e0)
            {
                // TODO:
                return false;
            }
            catch (NullReferenceException e1)
            {
                // TODO:
                return false;
            }
            catch (OverflowException e2)
            {
                // TODO:
                return false;
            }
            catch (Exception e3)
            {
                // TODO:
                return false;
            }

            return true;
        }

        public byte GetPacketType(byte[] packet)
        {
            if (IsPacketValid(packet))
                return packet[typeLoc];
            else
                throw new ArgumentException("packet", "type is not valid");
        }

        public byte[] UnpackData(byte[] packet)
        {
            if (IsPacketValid(packet))
            {
                short len = (short)(packet[lenMSBLoc] & 0xff);
                len <<= 8;
                len |= (short)(packet[lenLSBLoc] & 0xff);

                byte[] rtn = new byte[len];

                for (short i = 0; i < len; i++)
                    rtn[i] = packet[dataStartLoc + i];

                return rtn;
            }
            else
                throw new ArgumentException("packet", "packet is not valid");
        }
    }
}
