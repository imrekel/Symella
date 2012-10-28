/*****************************************************************************
 * Copyright (C) 2005, 2006 Imre Kelényi, Bertalan Forstner
 *-------------------------------------------------------------------
 * This file is part of Symella
 *
 * Symella is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Symella is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Symella; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#ifndef SAPACKET_H
#define SAPACKET_H

#include <e32def.h>
#include <e32des8.h>
#include <in_sock.h>

#define PACKET_PING 0x00
#define PACKET_PONG 0x01
#define PACKET_BYE	0x02
#define PACKET_PUSH	0x40
#define PACKET_QUERY 0x80
#define PACKET_QUERY_HIT 0x81
#define PACKET_VENDOR_MESSAGE 0x31

class TInterval
{
public:
	TInterval()
	{
		iStart = 0;
		iEnd = 0;
	}

	TInterval(TUint aStart, TUint aEnd)
	{
		iStart = aStart;
		iEnd = aEnd;
	}

	TBool operator==(const TInterval& aInterval)
	{
		if ((iStart == aInterval.iStart)&&
			(iEnd == aInterval.iEnd))
			return ETrue;
		return EFalse;
	}

	TInterval(const TInterval& aInterval)
	{
		*this = aInterval;
	}

	TBool operator!=(const TInterval& aInterval)
	{
		return !(*this == aInterval);
	}

	const TInterval& operator=(const TInterval& aInterval)
	{
		iStart = aInterval.iStart;
		iEnd = aInterval.iEnd;

		return *this;
	}

public:
	TUint iStart;
	TUint iEnd;
};

class TGuid
{
public:

	TGuid()
	{
		TInt i;
		for (i=0;i < 16;i++)
			iGuid[i] = 0x00;

		//set the two important bytes
		iGuid[8]  = 0xFF;
		iGuid[15] = 0x00;
	}

	TGuid(const TGuid& aGuid)
	{
		*this = aGuid;
	}

	TBool operator==(const TGuid& aGuid)
	{
		for (TInt i=0; i<16; i++)
			if (iGuid[i] != aGuid.iGuid[i]) return EFalse;

		return ETrue;
	}

	TBool operator!=(const TGuid& aGuid)
	{
		return !(*this == aGuid);
	}

	const TGuid& operator=(const TGuid& aGuid)
	{
		for (TInt i=0; i<16; i++)
			iGuid[i] = aGuid.iGuid[i];

		return *this;
	}

public:

	TUint8 iGuid[16];

	//TGuid(TDes8& aGuidBuf)
	//{
	//	TInt i;
	//	TInt count=16;
	//	if (aGuidBuf.Length() < 16)
	//		count = aGuidBuf.Length();

	//	for (i=0; i < count; i++)
	//		iGuid[i] = aGuidBuf[i];

	//	//if the buffer is smaller than 16, fill the rest up with 0
	//	if (count<16)
	//		for (i=count;i<16;i++)
	//			iGuid[i] = aGuidBuf[i];
	//}
};

//struct TPacketHeader
//{
//	TGuid	iGuid;		//0-15
//	TUint8	iFunction;	//16
//	TUint8	iTTL;		//17
//	TUint8	iHops;		//18
//	TUint32	iPayLoad;	//19-22
//};
//
//struct TPongPayLoad
//{
//	TUint16	iPort;		//23-24
//	TInetAddr		iInetAddr;		//25-28
//	TUint32	iFileCount;	//29-32
//	TUint32	iFileSize;	//33-36
//}

#endif