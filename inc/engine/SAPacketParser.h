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

#ifndef SAPACKETPARSER_H
#define SAPACKETPARSER_H

#include <coemain.h>
#include <e32des8.h>
#include <e32std.h>
#include <in_sock.h>

class CSANode;

struct TPacketHeader;
class TGuid;

const TUid KUidSymellaPacketParser = { 0x10000EF2 };
const TUint8 KQueryHitsExpected = 5;

#define PACKETPARSER CSAPacketParser::InstanceL()
#define PongsToPing				2
#define KWrongPayLoadLength		-10
#define	KNegativeHops			-11
#define KNegativeTTL			-12
#define KWrongPayLoadType		-13

class CSAPacketParser : public CCoeStatic
{
public:
	static CSAPacketParser* InstanceL();

	~CSAPacketParser();

	void ConstructL();

	void ResetRandomSeed();
	TGuid GenerateGuid();
//	TGuid* GenerateGuidL();

	//sender functions
	TBuf8<23> CreatePacketHeader(const TGuid& aGuid,const TUint8 aFunction,const TUint8 aTTL,const TUint8 aHops,const TUint32 aPayLoad);
	void CreatePacketHeader(TDes8& aBuffer,const TGuid& aGuid,const TUint8 aFunction,const TUint8 aTTL,const TUint8 aHops,const TUint32 aPayLoad);
	//void SendPingPacket(CSANode& aNode,const TGuid& aGuid);
	void SendPingPacketL(CSANode& aNode);
	void SendPongPacket(CSANode& aNode,const TGuid& aGuid,const TInetAddr& aInetAddr,const TUint16 aPort,const TUint32 aSharedFiles,const TUint32 aSharedAmount);
	void SendPushPacket(CSANode& aNode,const TGuid& aGuid,const TGuid& aServerGuid,const TUint32 aIndex,const TInetAddr aInetAddr,const TUint16 aPort);
	void SendQueryPacketL(CSANode& aNode, const TGuid& aGuid, TUint16 aMinSpeed, const TDesC8& aQuery);
	void SendByePacketL(CSANode& aNode,const TGuid& aGuid,const TUint16 aErrorCode,const TDesC8& aReason);
	void SendStopSearchPacketL(CSANode& aNode,const TGuid& aGuid);
	//CreateQueryResult function is not necessary, we won't answer to Query messages - we do not have any resources shared

	//Logger function
	void LogPacketData(const TDesC8& aPacket);

	//parser function
	TInt ParsePacketL(CSANode& aNode,const TDesC8& aHeader,const TDesC8& aPayLoad);

	//handler functions
	void HandleIncomingPing(CSANode& aNode,const TGuid& aGuid,const TUint8 aTTL,const TUint8 aHops);
	void HandleIncomingPong(CSANode& aNode,TInetAddr& aInetAddr,const TUint16 aPort,const TUint32 aSharedFiles,const TUint32 aSharedAmount, const TDesC8& aPayLoad);
	void HandleIncomingBye(CSANode& aNode,const TUint16 aErrorCode,const TDesC8& aReason);
	void HandleIncomingPush(CSANode& aNode,const TGuid& aServerGuid,const TUint32 aIndex,const TInetAddr& aInetAddr,const TUint16 aPort);
	void HandleIncomingQueryHitL(CSANode& aNode,TInetAddr& aInetAddr,const TUint16 aPort,const TUint32 aSpeed, const TDesC8& aPayLoad, TInt aGGEPPos, const TUint8 aHops);

protected:
	CSAPacketParser() : CCoeStatic(KUidSymellaPacketParser) {}

	TInt64	iRandomSeed;
};

#endif