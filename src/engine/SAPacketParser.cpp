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

#include <e32math.h>
#include "SAPacketParser.h"
#include "SAPacket.h"
#include "SALogger.h"
#include <e32base.h>
#include "SANode.h"
#include "SAControl.h"
#include "SADownloadManager.h"
#include "SAHostCache.h"
#include "SAMessageCache.h"
#include "SAHash.h"
#include <aknnotewrappers.h> 

_LIT8(KUrnSha1, "urn:sha1:");
_LIT8(KDefaultHash, "00000000000000000000000000000000");

CSAPacketParser* CSAPacketParser::InstanceL()
{
	CSAPacketParser* instance = static_cast<CSAPacketParser*> 
		( CCoeEnv::Static( KUidSymellaPacketParser ) );
	
	if (instance == 0)
	{
		instance = new (ELeave) CSAPacketParser();

		CleanupStack::PushL( instance ); 
		instance->ConstructL(); 
		CleanupStack::Pop(); 
	}

	return instance;
}

void CSAPacketParser::ConstructL()
{
	ResetRandomSeed();
}

void CSAPacketParser::ResetRandomSeed()
{	
	iRandomSeed = TInt(Math::Random());
	iRandomSeed *= iRandomSeed;
}

CSAPacketParser::~CSAPacketParser()
{
}

TGuid CSAPacketParser::GenerateGuid()
{
	TGuid guid;
	TInt16 a;
	for (TInt i=0;i<8;i++)
	{
		a = (TInt16)Math::Random();
		guid.iGuid[2*i] = TUint8(a & 0x00FF);
		guid.iGuid[2*i+1] = TUint8((a & 0xFF00) >> 8);
	}
	guid.iGuid[8] = 0xFF; //show that we are a modern servent
	guid.iGuid[15] = 0x00; //reserved for future use
	return guid;
}

/*TGuid* CSAPacketParser::GenerateGuidL()
{
	TGuid* guid = new (ELeave) TGuid();
	TInt16 a;
	for (TInt i=0;i<8;i++)
	{
		a=Math::Rand(iRandomSeed);
		guid->iGuid[2*i] = a & 0x00FF;
		guid->iGuid[2*i+1]=(a & 0xFF00) >> 8;
	}
	guid->iGuid[8] = 0xFF; //show that we are a modern servent
	guid->iGuid[15] = 0x00; //reserved for future use

	return guid;
}*/

void CSAPacketParser::SendPingPacketL(CSANode& aNode)
{
	TInetAddr addr;
	//if (CTR->GetLocalAddress(addr)==KErrNone)
	//{
		//generate guid and put it in the messagecache
		TGuid guid = GenerateGuid();		
		CTR->MessageCache().AddGuidL(guid);		

		//create the header
		TBuf8<23> buf;
		CreatePacketHeader(buf, guid, PACKET_PING, 2, 0, 0);
		//send the packet, it'll be the ping
		//TODO: it is commented out, don't know whether it is necessary to put it in
		aNode.SendL(buf);
	
		LOG->WriteLineL(_L("[OUT] Ping sent."));

	//}
}

TBuf8<23> CSAPacketParser::CreatePacketHeader(const TGuid& aGuid,const TUint8 aFunction,const TUint8 aTTL,const TUint8 aHops,const TUint32 aPayLoad)
{
	//create 23 bytes in the buffer
	TBuf8<23> header;

	//initialize
	header.Zero();

	//put the Guid in it
	for (TInt i=0;i<16;i++)
	{
		header.Append(aGuid.iGuid[i]);
	}

	//put the PACKET_XXX in it
	header.Append(aFunction);

	//TTL is X
	header.Append(aTTL);

	//Hops is X, because we'll start it
	header.Append(aHops);

	//payload size is X
	header.Append((aPayLoad & 0x000000FF));
	header.Append((aPayLoad & 0x0000FF00) >> 8);
	header.Append((aPayLoad & 0x00FF0000) >> 16);
	header.Append((aPayLoad & 0xFF000000) >> 24);
	
	return header;
}

void CSAPacketParser::CreatePacketHeader(TDes8& aBuffer,const TGuid& aGuid,const TUint8 aFunction,const TUint8 aTTL,const TUint8 aHops,const TUint32 aPayLoad)
{
	//initialize
	aBuffer.Zero();

	//put the Guid in it
	for (TInt i=0;i<16;i++)
	{
		aBuffer.Append(aGuid.iGuid[i]);
	}

	//put the PACKET_XXX in it
	aBuffer.Append(aFunction);

	//TTL is X
	aBuffer.Append(aTTL);

	//Hops is X
	aBuffer.Append(aHops);

	//payload size is X
	aBuffer.Append((aPayLoad & 0x000000FF));
	aBuffer.Append((aPayLoad & 0x0000FF00) >> 8);
	aBuffer.Append((aPayLoad & 0x00FF0000) >> 16);
	aBuffer.Append((aPayLoad & 0xFF000000) >> 24);
}

void CSAPacketParser::SendPongPacket(CSANode& aNode,const TGuid& aGuid,const TInetAddr& aInetAddr,const TUint16 aPort,const TUint32 aSharedFiles,const TUint32 aSharedAmount)
{

    //count the packet length =  + 1 (0x00 closing char)
	TInt length = 37;
	HBufC8 * msg = HBufC8::NewL(length);
	CleanupStack::PushL(msg);
	TPtr8 pong = msg->Des();

		CreatePacketHeader(pong,aGuid,PACKET_PONG,KMaxTTL,0,14);
	
	//copy the header to the start of the packet
	//pong.Copy(header);

	//copy the port to the descriptor
	pong.Append(aPort & 0x00FF);
	pong.Append((aPort & 0xFF00) >> 8);

	//copy the address to the descriptor
	TUint32 address = aInetAddr.Address();
	pong.Append((address & 0xFF000000) >> 24);
	pong.Append((address & 0x00FF0000) >> 16);
	pong.Append((address & 0x0000FF00) >> 8);
	pong.Append((address & 0x000000FF));

	//copy the shared files counter to the descriptor
	pong.Append((aSharedFiles & 0x000000FF));
	pong.Append((aSharedFiles & 0x0000FF00) >> 8);
	pong.Append((aSharedFiles & 0x00FF0000) >> 16);
	pong.Append((aSharedFiles & 0xFF000000) >> 24);

	//copy the shared kb-s counter to the packet
	pong.Append((aSharedAmount & 0x000000FF));
	pong.Append((aSharedAmount & 0x0000FF00) >> 8);
	pong.Append((aSharedAmount & 0x00FF0000) >> 16);
	pong.Append((aSharedAmount & 0xFF000000) >> 24);

	//send
	aNode.SendL(pong);

	//log
	LOG->WriteLineL(_L("[OUT] Pong sent."));
	//LogPacketData(pong);
//		CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
//		note->ExecuteLD(_L("Pong sent."));


		CleanupStack::PopAndDestroy(); //msg
}

void CSAPacketParser::SendStopSearchPacketL(CSANode& aNode,const TGuid& aGuid)
{
	TBuf8<33> buf;
	CreatePacketHeader(buf,aGuid,PACKET_VENDOR_MESSAGE,1,0,10);
	//vendor id
	buf.Append(0x52); //R [0]
	buf.Append(0x41); //A [1]
	buf.Append(0x45); //E [2]
	buf.Append(0x42); //B [3]
	//sub-selector
	buf.Append(0x0C); //0x000C [4]
	buf.Append(0x00); //0x000C [5]
	//version
	buf.Append(0x01); // 0x01 [6]
	buf.Append(0x00); //  [7]
	//hit count
	buf.Append(0xFF); //0xFFFF [8]
	buf.Append(0xFF); //0xFFFF [9]

	aNode.SendL(buf);

	//log
	LOG->WriteLineL(_L("[OUT]: stop search packet sent"));
}

void CSAPacketParser::SendPushPacket(CSANode& aNode,const TGuid& aGuid,const TGuid& aServerGuid,const TUint32 aIndex,const TInetAddr aInetAddr,const TUint16 aPort)
{
	//TBuf8<23> header = CreatePacketHeader(aGuid,0x40,7,0,26);
	TBuf8<49> push;

	//copy the header to the start of the packet
	CreatePacketHeader(aGuid,PACKET_PUSH,3,0,26);
	//push.Copy(header);

	//put the serverID to the packet
	for (TInt i=0;i<16;i++)
		push.Append(aServerGuid.iGuid[i]);

	//put the file index afterwards
	push.Append((aIndex & 0x000000FF));
	push.Append((aIndex & 0x0000FF00) >> 8);
	push.Append((aIndex & 0x00FF0000) >> 16);
	push.Append((aIndex & 0xFF000000) >> 24);

	//put the address to the descriptor
	TUint32 address = aInetAddr.Address();
	push.Append((address & 0xFF000000) >> 24);
	push.Append((address & 0x00FF0000) >> 16);
	push.Append((address & 0x0000FF00) >> 8);
	push.Append((address & 0x000000FF));

	//put the port in the packet
	push.Append((aPort & 0x00FF));
	push.Append((aPort & 0xFF00) >> 8);

	//send
	aNode.SendL(push);

	//log
	LOG->WriteLineL(_L("[OUT] Push sent."));
	//LogPacketData(push);
}

void CSAPacketParser::SendQueryPacketL(CSANode& aNode, const TGuid& aGuid, TUint16 /*aMinSpeed*/, const TDesC8& aQuery)
{
	//count the packet length = 2 (minimal speed) + query length + 1 (0x00 closing char)
	TInt length = 2 + aQuery.Length() + 1;
	HBufC8 * query = HBufC8::NewL(23 + length);
	CleanupStack::PushL(query);
	TPtr8 des = query->Des();

	//TBuf8<23> header = CreatePacketHeader(aGuid,0x80,7,0,length);
	
	//copy the header to the packet
	//des.Copy(header);
	CreatePacketHeader(des,aGuid,PACKET_QUERY,3,0,length);

	//put a 16-bit flagpacket in the place of the minimalspeed
	TUint8 fstbyte = 0;
	TUint8 scdbyte = 0;
	//show that we are using flags (15th bit)
	fstbyte = fstbyte || (0x01 << 7);
	//we are not using a firewall (14th bit)
	fstbyte = fstbyte || (0x01 << 6);
	//we are not accepting XML metadata (13th bit)
	fstbyte = fstbyte || (0x01 << 5);
	//we are not using dynamic query (12th bit)
	fstbyte = fstbyte || (0x01 << 4);
	//we are not accepting GGEP H extensions (11th bit)
	fstbyte = fstbyte || (0x01 << 3);
	//we are not sending data out-of-band (10th bit)
	fstbyte = fstbyte || (0x01 << 2);
	//reserved bit (9th bit)
	//upper bit of the max return queryhits (8th bit)
	//queryhit
	scdbyte = KQueryHitsExpected;
	des.Append(fstbyte);
	des.Append(scdbyte);

	//put the minimal speed to the packet
	//des.Append((aMinSpeed & 0x00FF));
	//des.Append((aMinSpeed & 0xFF00) >> 8);

	//put the query to the packet
	des.Append(aQuery);

	//for (TInt i=0;i<aQuery.Length();i++)
	//	des.Append(aQuery[i]);

	//put a closing 0x00 to the query
	des.Append(0x00);

	//send the query
	aNode.SendL(*query);

	//log
	LOG->WriteL(_L("[OUT] Query sent:"));
	LOG->WriteLineL(aQuery);
	//LogPacketData(*query);
	//destroy HBufC
	CleanupStack::PopAndDestroy(); //query
}

void CSAPacketParser::SendByePacketL(CSANode& aNode,const TGuid& aGuid,const TUint16 aErrorCode,const TDesC8& aReason)
{
	//count the packet length = 2 (code) + reason length + 1 (null termination)
	//Bye packet, from the draft: TTL 1, Hops 0
	TInt length = 2 + aReason.Length() + 1;
	//TBufC8<23> header = CreatePacketHeader(aGuid,0x02,1,0,length);
	HBufC8 * bye = HBufC8::NewL(23 + length);
	CleanupStack::PushL(bye);
	TPtr8 des = bye->Des();

	//copy the header to the packet
	//des.Copy(header);
	CreatePacketHeader(des,aGuid,PACKET_BYE,1,0,length);

	//put the code to the packet
	des.Append((aErrorCode & 0x00FF));
	des.Append((aErrorCode & 0xFF00) >> 8);

	//put the reason to the end of the packet
	for (TInt i=0;i<aReason.Length();i++)
		des.Append(aReason[i]);

	//close the string
	des.Append(0x00);

	//send the query
	aNode.SendL(*bye);

	//log
	LOG->WriteLineL(_L("[OUT] Bye sent"));
	//LogPacketData(*bye);
	//destroy
	CleanupStack::PopAndDestroy(); //query
}

void CSAPacketParser::LogPacketData(const TDesC8& aPacket)
{
	//TBuf<300> packet;
	//packet.Zero();
	//packet.Copy(aPacket);
	//write out the packet string
	//LOG->WriteLineL(packet);
	//write out the byte values
	TBuf<15> buf;
	buf.Zero();
	for (TInt i=0;i<aPacket.Length();i++)
	{
		buf.Zero();
		buf.Format(_L("[%d]%d"),i,aPacket[i]);
		LOG->WriteLineL(buf);		
	}
}

TInt CSAPacketParser::ParsePacketL(CSANode& aNode,const TDesC8& aHeader,const TDesC8& aPayLoad)
{
	//variables
	TInt result = KErrNone;

	//header variables
	TGuid guid;
	TUint8 payloadType;
	TUint8 ttl;
	TUint8 hops;
	TUint32 payloadSize;

	//get the message guid
	for (TInt i=0;i<16;i++)
		guid.iGuid[i] = aHeader[i];
	//get the payloadType
	payloadType = aHeader[16];
	//get the ttl
	ttl = aHeader[17];
	//get the hops
	hops = aHeader[18];
	//get the payloadSize
	payloadSize = (aHeader[22] << 24) | (aHeader[21] << 16) | (aHeader[20] << 8) | (aHeader[19]);

	//checks based on the header data from LimeWire
	//if (payloadSize<0) {result = KWrongPayLoadLength;}
	//if (hops<0) {result = KNegativeHops;}
	//if (ttl<0) {result = KNegativeTTL;}
	if ((payloadType!=PACKET_PING)&&(payloadType!=PACKET_PONG)&&(payloadType!=PACKET_PUSH)&&
		(payloadType!=PACKET_BYE)&&(payloadType!=PACKET_QUERY)&&(payloadType!=PACKET_QUERY_HIT))
	{result = KWrongPayLoadType;}

	//continue only if there was no error parsing the header
	if (result == KErrNone)
	{
		//continue with the correct parsing based on the payloadType
		switch (payloadType)
		{
		case PACKET_PING:
			{
				//PING
				//handle the ping request
				HandleIncomingPing(aNode,guid,ttl,hops);
			}
			break;
		case PACKET_PONG:
			{
				//PONG
				//Pong payload variables
				TUint16 port;
				TUint32 ipn;
				TUint32 sharedFiles;
				TUint32 sharedAmount;
				//parse the pong payload
				//port
				port = TUint16((aPayLoad[0]) | (aPayLoad[1] << 8));
				//ip
				ipn = (aPayLoad[2] << 24) | (aPayLoad[3] << 16) | (aPayLoad[4] << 8) | (aPayLoad[5]);
				TInetAddr ip;
				ip.SetAddress(ipn);
				ip.SetPort(port);
				//shared files
				sharedFiles = (aPayLoad[6]) | (aPayLoad[7] << 8) | (aPayLoad[8] << 16) | (aPayLoad[9] << 24);
				//shared amount
				sharedAmount = (aPayLoad[10]) | (aPayLoad[11] << 8) | (aPayLoad[12] << 16) | (aPayLoad[13] << 24);

				//handle the incoming pong
				HandleIncomingPong(aNode,ip,port,sharedFiles,sharedAmount, aPayLoad);
			}
			break;
		case PACKET_BYE:
			{
				//BYE
				//bye payload variables
				TUint16 errorCode;
				HBufC8* reason = HBufC8::NewL(aPayLoad.Length()-2);
				CleanupStack::PushL(reason);
				TPtr8 des = reason->Des();

				//parse them
				errorCode = TInt16((aPayLoad[0]) | (aPayLoad[1] << 8));
				//copy the rightmost characters to the reason, leave only the first 2 bytes
				des.Copy(aPayLoad.Right(aPayLoad.Length()-2));

				//handle the incoming bye 
				HandleIncomingBye(aNode,errorCode,*reason);
				//destroy reason
				CleanupStack::PopAndDestroy();
			}
			break;
		case PACKET_PUSH:
			{
				//PUSH
				//push payload variables
				TGuid servent;
				TUint32 fileIndex;
				TUint32 ipn;
				TUint16 port;
				
				//parse them
				//servent guid
				for (TInt i=0;i<16;i++)
					servent.iGuid[i] = aPayLoad[i];
				//file index
				fileIndex = (aPayLoad[16]) | (aPayLoad[17] << 8) | (aPayLoad[18] << 16) | (aPayLoad[19] << 24);
				//ip address
				ipn = (aPayLoad[20] << 24) | (aPayLoad[21] << 16) | (aPayLoad[22] << 8) | (aPayLoad[23]);
				TInetAddr ip(ipn,80);
				//port
				port = TInt16((aPayLoad[24]) | (aPayLoad[25] << 8));

				//handle the request
				HandleIncomingPush(aNode,servent,fileIndex,ip,port);
			}
			break;
		case PACKET_QUERY:
			{
				//QUERY
				//TODO: we don't answer to querys, because we don't have anything to share
				//and we don't want to pay for the downstream traffic
			}
			break;
		case PACKET_QUERY_HIT:
			{
				//log the payload
				//LOG->WriteL(_L("*"));
				//LOG->WriteL(aPayLoad);
				//test whether we have the guid of the incoming queryhit - when not, do nothing
				/*LOG->WriteL(guid.iGuid[0]);
				LOG->WriteL(guid.iGuid[1]);
				LOG->WriteL(guid.iGuid[2]);
				LOG->WriteL(guid.iGuid[3]);
				LOG->WriteLineL();*/
				
					TUint8 sndByte=0;
					TUint8 fstByte=0;				
				
				if (CTR->MessageCache().SearchGuid() != guid)
					break;

				//QUERY HIT
				//query hit variables
				TUint8 numberOfHits;
				TUint16 port;
				TUint32 ipn;
				TUint32 speed;

				//parse numberofhits
				numberOfHits = aPayLoad[0];
				//parse port
				port = TInt16((aPayLoad[1]) | (aPayLoad[2] << 8));
				//parse ip address
				ipn = (aPayLoad[3] << 24) | (aPayLoad[4] << 16) | (aPayLoad[5] << 8) | (aPayLoad[6]);
				//TInetAddr ip(ipn,port);
				//parse speed
				speed = (aPayLoad[7]) | (aPayLoad[8] << 8) | (aPayLoad[9] << 16) | (aPayLoad[10] << 24);

				//log them
				//TBuf<256> log;
				//TBuf<256> log2;
				TInetAddr ip2(ipn,port);


//				TBuf<20> ips;
//				ip.Output(ips);
//				log.Format(_L("[MESSAGE] QueryHit, %d hits from %S:"),numberOfHits,&ips);
//				LOG->WriteLineL(log);

				//parse files
				TUint32 counter = 11;
				TUint32 fileIndex;
				TUint32 fileSize;
				TUint	fileNameLength;
				TUint endOfFileInfo = 0;
				TInt urnshapos=0;
				TBuf8<32> hash;
				HBufC8* fileName;
				TInt GGEPPos = 0;

				//parse all the files that are in the packet
				for (TInt i=0;i<numberOfHits;i++)
				{
					//parse file index
					fileIndex = (aPayLoad[counter]) | (aPayLoad[counter+1] << 8) | (aPayLoad[counter+2] << 16) | (aPayLoad[counter+3] << 24);
					counter += 4;
					//parse file size
					fileSize = (aPayLoad[counter]) | (aPayLoad[counter+1] << 8) | (aPayLoad[counter+2] << 16) | (aPayLoad[counter+3] << 24);
					counter += 4;
					fileNameLength = 0;
					//get file name length, it lasts until a 0x00
					while (aPayLoad[counter+fileNameLength] != 0x00)
						fileNameLength++;
						
					//we've got the length,get out the filename
					fileName = HBufC8::NewLC(fileNameLength);
					
					TPtr8 des = fileName->Des();
					des.Copy(aPayLoad.Mid(counter,fileNameLength));
					counter = counter + fileNameLength;

					//we are at the 0x00 character - we skip the next part (extension block), 
					//until the next 0x00 character
					endOfFileInfo=counter+1;
					while (aPayLoad[endOfFileInfo]!=0x00)
						endOfFileInfo++;
					

					//if there is an extension, and is long enough to store an sha
					if (endOfFileInfo >= counter+32+9)
					{
						//find the hash
						TPtrC8 ext=aPayLoad.Mid(counter,endOfFileInfo-counter+1);

						urnshapos=ext.Find(KUrnSha1);
						if (urnshapos != KErrNotFound)
						{
							//get the hash out
							hash.Copy(aPayLoad.Mid(counter+urnshapos+9, 32));
						}
						else
						{
							hash.Copy(KDefaultHash);
						}
					}
					else
					{
						hash.Copy(KDefaultHash);
					}

					//we are ath the end of the extension block, increase the counter with one, to start parsing the next file
					counter=endOfFileInfo+1;

					//TODO: handle the incoming file hits
					//write it out to the log
					/*TBuf<256> log;
					TBuf<256> log2;
					log2.Copy(*fileName);
					log.Zero();
					log.Format(_L("[%d]%S,%d bytes"),fileIndex,&log2,fileSize);
					LOG->WriteLineL(log);*/

					///////////////////////////////

					CTR->DlManager()->RegisterQueryHitL(*fileName, fileSize, fileIndex, ipn, port, TSAHash(hash));

					///////////////////////////////

					//delete fileName,we don't need it anymore
					CleanupStack::PopAndDestroy(); //fileName
				}

				/*LOG->WriteLineL(_L("Rest of the payload:"));
				LOG->WriteLineL(aPayLoad.Mid(counter));*/
				//parse the rest of the queryhit only if it's larger than 23 (16 servent id+min extra block size
				if (aPayLoad.Length()-counter > 23)
				{
					//[0-3] is vendorcode, we don't really care about that
					if(aPayLoad[counter+4]==2 ) { //[4] is open data size, expecting 2
					//[5] open data first byte
					//[6] open data second byte
					
					TUint8 sndByte = aPayLoad[counter+6];
					TUint8 fstByte = aPayLoad[counter+5];
					

					LOG->WriteL(_L("fstByte:"));
					LOG->WriteLineL(fstByte);
					LOG->WriteL(_L("sndByte:"));
					LOG->WriteLineL(sndByte);
					
					//see if it's firewalled
					/*if (((fstByte & 0x01) == 0x01)&&((sndByte & 0x01) == 0x01))
					{
						LOG->WriteLineL(_L("Host is firewalled, needs a push"));
					}*/


					//is there GGEP?
					if((fstByte & 32)==32) 
					{

					GGEPPos = counter+7;


					} //GGEP OK
					}  //open data size OK
					//store the last 16 characters as a servent id
					TBuf<16> serventID;
					serventID.Copy(aPayLoad.Right(16));
					/*LOG->WriteL(_L("ServentID:"));
					LOG->WriteLineL(serventID);*/
				}
				HandleIncomingQueryHitL(aNode,ip2,port,speed, aPayLoad, GGEPPos, hops);
				
			}
			break;
		default:
			{
			//Not Gnutella supported header, or out of sync
				result = KWrongPayLoadType;
			}
			break;
		}
	}
	
	//return with the result
	return result;
}

void CSAPacketParser::HandleIncomingPing(CSANode& aNode,const TGuid& aGuid,const TUint8 /*aTTL*/,const TUint8 /*aHops*/)
{
	//send replies with the same guid, and larger TTL than the source ping and with 0 hops
	//Gnutella draft advises 10 ppongs, that is too much
	//use our IP address
	TInetAddr ip(0x9842D7D4,3636);
	//for (TInt i=0;i<PongsToPing;i++)
	SendPongPacket(aNode,aGuid,ip,4242,0,0);
	if(aNode.IsSemPeer()) {
		LOG->WriteL(_L("Incoming Semantic Ping."));

	}
	
}

void CSAPacketParser::HandleIncomingPong(CSANode& aNode,TInetAddr& aInetAddr,const TUint16 aPort,const TUint32 /*aSharedFiles*/,const TUint32 /*aSharedAmount*/, const TDesC8& aPayLoad)
{
	LOG->WriteL(_L("Incoming pong from:"));
	TBuf<40> buf;
	aNode.RemoteAddress().Output(buf);
	LOG->WriteL(buf);
	LOG->WriteL(_L(":"));
	LOG->WriteLineL(aInetAddr.Port());

	//put the ip to the unidentifiedList
	aInetAddr.SetPort(aPort);
	CTR->HostCache().AddUnidentifiedL(aInetAddr);

#ifdef EKA2

	if(aNode.IsSemPeer()) {
		//magic byte
		if(aPayLoad[14] == 195 &&
		//last ggep extension
		   aPayLoad[15] == 113 && 

		// ID = SemPeer
		   aPayLoad[16] == 83 && 
		   aPayLoad[17] == 101 && 
		   aPayLoad[18] == 109 && 
		   aPayLoad[19] == 80 && 
		   aPayLoad[20] == 101 && 
		   aPayLoad[21] == 101 && 
		   aPayLoad[22] == 114) {

				aNode.SetHasProfile(1);
				//size of extension
				TInt i=0;
				TUint32 length = 0;
				while (i==0 || /*the prevoius character*/ aPayLoad[22+i]&192 !=64 )
				{
					length = 64*length+(aPayLoad[23+i]&63);
					i++;
				}

			    //SemPeer payload is n*(8+4) bytes.
				if(length % 12 != 0) return;

				TInt64 key;

				TInt64 helpkey;
				TInt64 multi = 429496729;
				TInt32 count;
				TInt start = 23+i;
				//reading the semantic profile
				//delete old profile
				for (TInt i=0; i<256; i++)
	    				aNode.profile()->At(i)=0;
				aNode.SetDocCount(0);
				
				//get new profile data
				for(i=0; i<length; i+=12) {
				    helpkey = (aPayLoad[start+i] << 24) | 
						(aPayLoad[start+1+i] << 16) | 
						(aPayLoad[start+2+i] << 8) | 
						(aPayLoad[start+3+i]);
					key =
						(aPayLoad[start+4+i] << 24) | 
						(aPayLoad[start+5+i] << 16) | 
						(aPayLoad[start+6+i] << 8) | 
						(aPayLoad[start+7+i]);
					key = multi*helpkey+key;
				    count = 
						(aPayLoad[start+8+i] << 24) | 
						(aPayLoad[start+9+i] << 16) | 
						(aPayLoad[start+10+i] << 8) | 
						(aPayLoad[start+11+i]);

				aNode.profile()->At(key)=count;
				aNode.SetDocCount(count+aNode.DocCount());		
				}	
		}//sempeer ggep ok
	}//issempeer conn

#endif // EKA2

}

void CSAPacketParser::HandleIncomingBye(CSANode& /*aNode*/,const TUint16 /*aErrorCode*/,const TDesC8& /*aReason*/)
{
	//TODO: handle bye requests - close the connection with the node, put an information note up with the reason, etc, etc
}

void CSAPacketParser::HandleIncomingPush(CSANode& /*aNode*/,const TGuid& /*aServerGuid*/,const TUint32 /*aIndex*/,const TInetAddr& /*aInetAddr*/,const TUint16 /*aPort*/)
{
	//TODO: do we need to handle push messages?
}

void CSAPacketParser::HandleIncomingQueryHitL(CSANode& /*aNode*/,TInetAddr& aInetAddr,const TUint16 aPort,const TUint32 /*aSpeed*/, const TDesC8& aPayLoad, TInt aGGEPPos, const TUint8 aHops)
{

#ifdef EKA2
	if(aGGEPPos != 0 && aHops > 0)
			//magic byte
		if(aPayLoad[aGGEPPos/*14*/] == 195 &&
		//last ggep extension
		   aPayLoad[aGGEPPos+1] == 113 && 

		// ID = SemPeer
		   aPayLoad[aGGEPPos+2] == 83 && 
		   aPayLoad[aGGEPPos+3] == 101 && 
		   aPayLoad[aGGEPPos+4] == 109 && 
		   aPayLoad[aGGEPPos+5] == 80 && 
		   aPayLoad[aGGEPPos+6] == 101 && 
		   aPayLoad[aGGEPPos+7] == 101 && 
		   aPayLoad[aGGEPPos+8] == 114) {
		
				//size of extension
				TInt i=0;
				TUint32 length = 0;
				while (i==0 || /*the prevoius character*/  aPayLoad[aGGEPPos+8+i]&192 !=64 )
				{
					length = 64*length+(aPayLoad[aGGEPPos+9+i]&63);
					i++;
				}

			    //SemPeer payload is n*(8+4) bytes.
				if((length-4) % 12 != 0) return;

				TInt64 key;

				TInt64 helpkey;
				TInt64 multi = 429496729; //for creating LONGs...
				TInt32 count;
				TInt start = aGGEPPos+13+i;
				CArrayFixFlat<TInt>* profile = new CArrayFixFlat<TInt>(256);
				CleanupStack::PushL(profile);
				
				//reading the semantic profile
				//delete  profile
				for (TInt i=0; i<256; i++)
	    				profile->AppendL(0);
				TInt docCount = 0;
				
				TInt gnutellaPort = (aPayLoad[start-4] << 24) | 
						(aPayLoad[start-3] << 16) | 
						(aPayLoad[start-2] << 8) | 
						(aPayLoad[start-1]);				
				aInetAddr.SetPort(gnutellaPort);
				
				//get new profile data
				for(i=0; i<length-4; i+=12) {
				    helpkey = (aPayLoad[start+i] << 24) | 
						(aPayLoad[start+1+i] << 16) | 
						(aPayLoad[start+2+i] << 8) | 
						(aPayLoad[start+3+i]);
					key =
						(aPayLoad[start+4+i] << 24) | 
						(aPayLoad[start+5+i] << 16) | 
						(aPayLoad[start+6+i] << 8) | 
						(aPayLoad[start+7+i]);
					key = multi*helpkey+key;
				    count = 
						(aPayLoad[start+8+i] << 24) | 
						(aPayLoad[start+9+i] << 16) | 
						(aPayLoad[start+10+i] << 8) | 
						(aPayLoad[start+11+i]);

				profile->At(key)=count;
				docCount = count+docCount;
				}
				TInt similarity = CTR->CalculateSimilarity(profile, docCount);
				if (CTR->ConnectionCount()<KDefaultMaxConnectionCount || CTR->MinimumSimilarity() < similarity)
				{
					TInetAddr addr;
					CTR->GetLocalAddress(addr);
					TUint32 intlocal= addr.Address();
					//the 4. segment
					intlocal = intlocal % 256;
					TUint32 intremote= aInetAddr.Address();
					//the 4. segment
					intremote = intremote % 256;
					
					
					//Topology Check for Parallel Rings Topology(TM) :-)
					//TODO topology check omitted for testing purposes (remove "1 || ")
					if(1 ||  intlocal % KMaxTTL +1 == intremote % KMaxTTL ) 
					{
						//Fire semantic node with the smallest similarity, if any
						//TODO: Fire this only when the other connection was succesful
						if(CTR->ConnectionCount()==KDefaultMaxConnectionCount)
						  CTR->FireLessSimilarNode();
						//Force Connect to the node!	
						CTR->ConnectNodeL(aInetAddr);					
					}	
				}
				CleanupStack::PopAndDestroy(profile);		
		}//sempeer ggep ok

#endif // EKA2

	//put the ip to the unidentifiednodes list
	aInetAddr.SetPort(aPort);

	//If not already a neighbor, add to candidates
	//if (aHops >0)
	    CTR->HostCache().AddUnidentifiedL(aInetAddr);
}