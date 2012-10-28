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

#include "SANode.h"
#include "SALogger.h"
#include "WriteBuffer.h"
#include "SAControl.h"
#include "ReadBuffer.h"
#include "SA.pan"
#include "SocketReader.h"
#include "SAHostCache.h"
#include "SAPacketParser.h"
#include <aknnotewrappers.h> 

// Timeout values in seconds
const TUint KHostResolveTimeout		=	15;
const TUint KConnectTimeout			=	30;
const TUint KHandshakeTimeout		=	10;
const TUint KGeneralTimeout			=	120; // no data received
const TUint KPingInterval			=	30; // after this we must send a ping packet to the node

const TUint KMessageHeaderLength	=	23;



_LIT8(KHandshakeInvalid, "Invalid handshake");
_LIT8(KHandshakeBufferOwerflow, "Handshake buffer owerflow");
_LIT8(KHandshakeInvalidProtocolVersion, "Handshake failed, invalid protocol version");
_LIT8(KHandshakeHeaderParsingFailed, "Handshake failed, invalid headers");


_LIT8(KLitTrue, "True");
_LIT8(KLitFalse, "False");

// Messages sent during handshake
_LIT8(KHandshakeGnutellaConnect,		"GNUTELLA CONNECT/0.6\r\n");
_LIT8(KHandshakeGnutella200Ok,			"GNUTELLA/0.6 200 OK\r\n");
_LIT8(KHandshakeHeaderXUltrapeerFlase,	"X-Ultrapeer: False\r\n"); 
_LIT8(KHandshakeHeaderUserAgent,		"User-Agent: BearShare 4.7.2.2\r\n");
_LIT8(KHandshakeHeaderXMaxTTL,			"X-Max-TTL: 3\r\n");
_LIT8(KHandshakeHeaderXSemPeer,			"X-SemPeer: 0.1\r\n");
_LIT8(KHandshakeHeaderXQueryRouting,	"X-Query-Routing: 0.1\r\n");

_LIT8(KHandshakeErrorLeafMode,			"GNUTELLA/0.6 503 In leaf mode\r\n");

_LIT8(KHandshakeFieldXUltrapeer,		"X-Ultrapeer");
_LIT8(KHandshakeFieldXSemPeer,			"X-SemPeer");
_LIT8(KHandshakeFieldXTryUltrapeers,	"X-Try-Ultrapeers");
_LIT8(KHandshakeFieldUserAgent,			"User-Agent");
_LIT8(KHandshakeFieldRemoteIP,			"Remote-IP");

_LIT8(KHandshakeEnd,					"\r\n");


const TUint KMaxHandshakeBufferSize = 2048; // should lower this


CSANode::CSANode(CSAControl& aControl, CNetworkManager* aNetMgr) 
 : CSocketBase(aNetMgr), iState(ENotConnected), iControl(aControl) 
{
	iAlreadySearched = EFalse;
	iHasProfile = 0;
	iDocCount = 0;
	iSimilarity = 0;

}

CSANode::~CSANode()
{
	Cancel();

	delete iNameEntry;	
	delete iProfile;

	/*if (iState != ENotConnected)
	{
		// we have to shut dwon manually (RSocket::Close() is not enough?)
		iSocket.Shutdown(RSocket::ENormal, iStatus);
		User::WaitForRequest(iStatus);
	}*/
}

void CSANode::ConstructL()
{
	CSocketBase::ConstructL();

	iLog = CSALogger::InstanceL();

	iHostCache = &(iControl.HostCache());
	
	iProfile = new CArrayFixFlat<TInt>(256);
	//The array will have 0-255 items (genre no.) with 0 counter.
	for (TInt i=0; i<256; i++)
	    iProfile->AppendL(0);
}

void CSANode::ConnectL(const TInetAddr& aAddr)
{
    __ASSERT_DEBUG( (iState == ENotConnected) || (iState == EResolving), 
		User::Panic(KSAPanicNode, ESAAlreadyConnected));
	
	__ASSERT_ALWAYS(!IsActive(), 
		User::Panic(KSAPanicNode, ESAActiveObjectIsActive));  

	CTR->ConnectionObserver()->AddAddressL(aAddr, iSemPeer);
	CTR->ConnectionObserver()->SetStateL(aAddr,MSAConnectionObserver::EConnecting);
	iRemoteAddress = aAddr;
	iConnDir = EOutbound;
	ChangeState(EConnecting);
	LogL(_L("Connecting"));
	OpenSocketL();
}

/*void CSANode::ConnectL(const TDesC& aAddress, TUint aPort)
{
//TODO socket-connection freezing on emu
#ifdef WINSCW
return;
#endif
	__ASSERT_DEBUG((iState == ENotConnected), User::Panic(KSAPanicNode, ESAAlreadyConnected));

	iRemoteAddress.SetPort(aPort);
		
	if (iRemoteAddress.Input(aAddress) == KErrNone)
	{
		// already a valid ip		
		ConnectL(iRemoteAddress);
	}
	else
	{
		//iLog->WriteLineL(_L("Resolving hostname"));
		// Initiate DNS
	    User::LeaveIfError(iResolver.Open(iSocketServ, KAfInet, KProtocolInetTcp));

		iState = EResolving;

		iNameEntry = new (ELeave) TNameEntry;
		
	    // DNS request for name resolution
	    iResolver.GetByName(aAddress, *iNameEntry, iStatus);	    
	    SetActive();
	}
}*/

void CSANode::SocketOpenedL(TBool aResult, RSocket& /*aSocket*/)
{
	if (aResult)
	{
		Socket().Connect(iRemoteAddress, iStatus);
		SetActive();
	}
	else
	{
		CloseL(_L8("Failed to open socket..."));
	}
}


void CSANode::ChangeState(TNodeState aState)
{
	iState = aState;
	iEllapsedTime = 0;
}


void CSANode::OnReceiveL()
{
//	iLog->WriteLineL(_L("SANode OnReceiveL, incoming data:"));
//	iLog->WriteLineL(iRecvBuffer->Ptr());

	switch (iState)
	{
		case EHandshaking:		
		{
			// checking for buffer owerflow (handshake failed)
			if ( (iRecvBuffer->Size() < KMaxHandshakeBufferSize))
			{
				_LIT8(KHandshakeHeadersEnd, "\r\n\r\n");
				TInt offset;
				TPtr8 buf(iRecvBuffer->Ptr());			

				// check if the remote side has finished transfering
				if ( (offset = buf.Find(KHandshakeHeadersEnd) ) != KErrNotFound )
				{
					//iLog->WriteLineL();
					//iLog->WriteLineL(buf.Left(offset));
					//iLog->WriteLineL();
					//iLog->WriteLineL(_L("CSANode header came, parse it"));

					offset += 4;
//					iLog->WriteLineL(_L("[Parse begin] ---------------------------"));
					ParseHandshakeL(buf.Left(offset));

					//iLog->WriteLineL(_L("CSANode header parsed"));
//					iLog->WriteLineL(_L("[Parse end] -----------------------------"));
					iRecvBuffer->Delete(0, offset);
				}
			}
			else
			// handshake failed
			{
				iHostCache->DegradeUltrapeerL(iRemoteAddress);
				iHostCache->DegradeWorkingNodeL(iRemoteAddress);
				CloseL(KHandshakeBufferOwerflow);				
			}
		}
		break;

		case EGnutellaConnected:		
		{
			// reseting timeout counter
			iEllapsedTime = 0;

			// checking if a message header has been received (23 bytes)
			if (iRecvBuffer->Size() >= KMessageHeaderLength)
			{
				TPtr8 buf = iRecvBuffer->Ptr();

				// getting the payload length from the header
				TUint32 payloadLength = (buf[22] << 24) | (buf[21] << 16) | (buf[20] << 8) | (buf[19]);
				
				// if we have a full message then parse it
				if ((TUint)iRecvBuffer->Size() >= KMessageHeaderLength + payloadLength)
				{
					//try to parse the packet
					if (payloadLength == 0)
						PACKETPARSER->ParsePacketL(*this,buf.Left(KMessageHeaderLength), TPtrC8());
					else
						PACKETPARSER->ParsePacketL(*this, buf.Left(KMessageHeaderLength), 
							buf.Mid(KMessageHeaderLength, payloadLength) );
					
//					iLog->WriteL(_L("[Msg] PT:"));
//					iLog->WriteL(buf[16]);
//					iLog->WriteL(_L(" PL:"));
//					iLog->WriteLineL(payloadLength);
					//if (buf[16] == 129) iLog->WriteL(_L("*"));

					// the message has been processed, we can delete it from the buffer
					iRecvBuffer->Delete(0, KMessageHeaderLength + payloadLength);

				}
			}


		}
		break;

		default:;
		break;
	}
}


void CSANode::ParseHandshakeL(const TDesC8& aBuf)
{
	//iLog->WriteLineL(aBuf);
	

	_LIT8(KGnutella, "GNUTELLA/"); // length = 9
	if ((aBuf.Length() < 16) || (aBuf.Left(9).Compare(KGnutella) != 0))
	{
		CloseL(KHandshakeInvalid);
		return;
	}

	TLex8 lex(aBuf);

	// extracting protocol version, we only accept 0.x where
	// x is higher then our version (currently 6)
	lex.Inc(11); // GNUTELLA/0.~6~
	lex.Mark();
	TInt version;	
	if ( (lex.Val(version) != KErrNone) || (version < KGnutellaProtocolVersion))
	{
		CloseL(KHandshakeInvalidProtocolVersion);
		return;
	}

	// checking "200"
	lex.Inc();
	lex.Mark();
	while (lex.Peek().IsDigit()) lex.Inc();

	// stores if the remote host accepts our connectin
	TBool connectAccepted = EFalse;
	_LIT8(K200, "200");
	//iLog->WriteLineL(_L("CSANode, Check 200"));
	if (lex.MarkedToken().Compare(K200) == 0)
	{
		connectAccepted = ETrue;
		//iLog->WriteLineL(_L8("Connection accepted!"));
	}
	else
	{
		//iLog->WriteLineL(_L("CSANode, not 200"));
		lex.Mark();
		while (lex.Peek() !='\r') lex.Inc();
		iLog->WriteLineL(lex.MarkedToken());
	}

	//proceeding to the first header
	while (lex.Get() != '\n')
	{		
	}

	lex.Mark();
	TPtrC8 fieldName;
	TBool parsingFailed = EFalse;
	// parsing headers
	while(lex.RemainderFromMark().Compare(KHandshakeEnd) != 0)
	{
		// trying to get the next header name
		if (ReadHeaderFieldName(lex, fieldName) != KErrNone)
		{
			parsingFailed = ETrue;			
			break;
		}

		//iLog->WriteL(_L("Parsed header name:"));
		//iLog->WriteLineL(fieldName);

		// X-Try-Ultrapeers
		if (fieldName.CompareF(KHandshakeFieldXTryUltrapeers) == 0)
		{
			if (ParseHeaderXTryUltrapeersL(lex) != KErrNone)
			{
				parsingFailed = EFalse;
				break;
			}
//			iLog->WriteLineL(_L8("Parsed x-try-ultrapeers"));
		}
		else

		// Remote-IP
		if (fieldName.CompareF(KHandshakeFieldRemoteIP) == 0)
		{
			if (ParseHeaderRemoteIP(lex) != KErrNone)
			{
				parsingFailed = EFalse;
				break;
			}
		}
		else
		
		// User-Agent
		if (fieldName.CompareF(KHandshakeFieldUserAgent) == 0)
		{
//			iLog->WriteLineL(_L8("Parsed user-agent"));
		}
		else

			// here come the headers that we are only intrested in
			// if the connection is accepted
			if (connectAccepted)
			{
				
				// SemPeer
				if (fieldName.CompareF(KHandshakeFieldXSemPeer) == 0)
				{					
					if (ParseHeaderSemPeerVersion(lex) != KErrNone)
					{
						parsingFailed = EFalse;
						break;
					}
					iLog->WriteLineL(_L8("Parsed SemPeer Header."));
					iSemPeer = ETrue;
				}

				
				// X-Ultrapeer
				if (fieldName.CompareF(KHandshakeFieldXUltrapeer) == 0)
				{					
					if (ParseHeaderXUltrapeer(lex) != KErrNone)
					{
						parsingFailed = EFalse;
						break;
					}
//					iLog->WriteLineL(_L8("Parsed x-ultrapeer"));
				}
			}			


		//proceeding to the next header
		while (lex.Get() != '\n')
		{			
		}
		
		lex.Mark();
	} // while


	if (parsingFailed)
	{
		CloseL(KHandshakeHeaderParsingFailed);
		iHostCache->RemoveUltrapeer(iRemoteAddress);
		//iHostCache->DegradeUltrapeerL(iRemoteAddress);
		//iLog->WriteLineL(_L8("PARSE FAILED"));
	}
	else
	{
		//iLog->WriteLineL(_L8("PARSE COMPLETE"));

		if (connectAccepted)
		{
			if (iUltrapeer)
			{
				SendL(KHandshakeGnutella200Ok);
				SendL(KHandshakeEnd);

				// We are connected to the node!
				ChangeState(EGnutellaConnected);
				CTR->ConnectionObserver()->SetStateL(iRemoteAddress,MSAConnectionObserver::EConnected);
				iLog->WriteLineL(_L8(" + GNUTELLA NODE CONNECTED!"));
				User::After(3000);
				PACKETPARSER->SendPingPacketL(*this);				
			}
			else
			{

				SendL(KHandshakeErrorLeafMode);
				SendL(KHandshakeEnd);

				CloseL(_L8("In leaf mode"));
				iHostCache->SetLeafL(iRemoteAddress);
				// disconnect, only accepting ultrapeers
			}
		}
		else
		{
			//LOG->WriteL(_L("["));
			//LOG->WriteL(aBuf);
			//LOG->WriteL(_L("]"));
			CloseL(_L8("Connection rejected"));
			iHostCache->DegradeUltrapeerL(iRemoteAddress);
			iHostCache->DegradeWorkingNodeL(iRemoteAddress);
		}
	}
}


TInt CSANode::ReadHeaderFieldName(TLex8& aLex, TPtrC8& aHeaderName)
{
	aLex.SkipSpace();
	aLex.Mark();

	// searching for ':'
	while (aLex.Peek() && (aLex.Peek() != ':'))
	{
		aLex.Inc();
	}

	if (aLex.Peek() == ':')
	{
		aHeaderName.Set(aLex.MarkedToken());

		// passing ':' and whitespace
		aLex.Inc();
		aLex.SkipSpace();
		return KErrNone;
	}
	else
		return KErrGeneral;	
}


TInt CSANode::ParseHeaderXTryUltrapeersL(TLex8& aLex)
{
	//iLog->WriteLineL(_L("CSANode ParseHeaderXTryUltrapeersL"));
//	iLog->WriteLineL(aLex.RemainderFromMark());
//	iLog->WriteLineL(_L("____________"));
	TChar c;
	TInt port = 0;
	TInetAddr addr;
	TBuf<16> ipBuf;

	aLex.SkipSpace();

	do
	{	
		while (!aLex.Peek().IsDigit())
		{
			aLex.Inc();
		}

		aLex.Mark();

		do
		{
			aLex.Inc();
			c = aLex.Peek();
		}
		while (c.IsDigit() || c=='.');

//		iLog->WriteL(aLex.MarkedToken());

		// checking ip format
		if (aLex.MarkedToken().Length() > 15)
			return KErrGeneral;
		ipBuf.Copy(aLex.MarkedToken());
		if (addr.Input(ipBuf) != KErrNone)				
			return KErrGeneral;

//		iLog->WriteL(_L(" : "));

		while (aLex.Get() != ':')
		{
		}
		
		aLex.SkipSpace();

//		iLog->WriteLineL(port);
		aLex.Mark();		
		// getting port number
		if (aLex.Val(port) != KErrNone)
			return KErrGeneral;
		addr.SetPort(port);

		//adding address to the host cache
		if ( !iControl.HostCache().IsUltrapeer(addr) )
			iHostCache->AddUltrapeerL(addr);

		// proceeding to the next address (or to the end of the header)
		do
		{
			c = aLex.Get();

			// checking for the end of handshake (\r\n\r\n sequence)
			if (c == '\r')
			{
				aLex.UnGet();
				aLex.Mark();
				aLex.Inc();
				if ( (aLex.Get() == '\n') && (aLex.Get() == '\r') && (aLex.Get() =='\n') )
				{
					aLex.UnGetToMark();
					break;
				}
				else
					aLex.UnGet();
			}			
		}
		while (!c.IsAlpha() && !c.IsDigit());

		aLex.UnGet();

		//c = aLex.Peek();
		if (c.IsAlpha() || (c == '\r') || (c=='\n'))
		{
			aLex.UnGet();
			return KErrNone;
		}

	}
	while (!aLex.Eos());

	return KErrGeneral;	
}


TInt CSANode::ParseHeaderRemoteIP(TLex8& aLex)
{
	//iLog->WriteLineL(_L("CSANode ParseHeaderRemoteIP"));

	aLex.SkipSpace();
	aLex.Mark();

	while (aLex.Peek() != '\r')
	{
		aLex.Inc();
	}

	if (aLex.MarkedToken().Length()>20)
		return KErrGeneral;

	TBuf<20> ipBuf;
	ipBuf.Copy(aLex.MarkedToken());

	TInetAddr addr;
	if (addr.Input(ipBuf) != KErrNone)
		return KErrGeneral;

	iControl.SetLocalAddress(addr);

	return KErrNone;
}


TInt CSANode::ParseHeaderSemPeerVersion(TLex8& aLex)
{
	iLog->WriteLineL(_L("CSANode ParseHeaderSemPeerVersion"));

	aLex.SkipSpace();
	aLex.Mark();

	while (aLex.Peek() != '\r')
	{
		aLex.Inc();
	}

	if (aLex.MarkedToken().Length()>4)
		return KErrGeneral;

	TBuf<4> verBuf;
	verBuf.Copy(aLex.MarkedToken());

	return KErrNone;
}


TInt CSANode::ParseHeaderXUltrapeer(TLex8& aLex)
{
	//iLog->WriteLineL(_L("CSANode ParseHeaderXUltrapeer"));

	aLex.SkipSpace();
	aLex.Mark();

	while (aLex.Peek() != '\r')
	{
		aLex.Inc();
	}

	if (aLex.MarkedToken().CompareF(KLitTrue) == 0)
	// ultrapeer
	{
		iUltrapeer = ETrue;
		CTR->ConnectionObserver()->SetUltrapeerL(iRemoteAddress);
		//iLog->WriteLineL(_L8("Ultrapeer!"));
	}
	else
		if (aLex.MarkedToken().CompareF(KLitFalse) == 0)
		// not ultrapeer
		{
			// disconnect
			//iLog->WriteLineL(_L8("Not Ultrapeer!"));
		}
		else
			return KErrGeneral;

	return KErrNone;
}


void CSANode::OnTimerL()
{
	//iLog->WriteL(_L("SANode OnTimerL, state:"));
	/*switch (iState)
	{
		case EResolving: iLog->WriteLineL(_L("EResolving")); break;
		case EConnecting: iLog->WriteLineL(_L("EConnecting")); break;
		case EHandshaking: iLog->WriteLineL(_L("EHandshaking")); break;
		case EGnutellaConnected: iLog->WriteLineL(_L("EGnutellaConnected")); break;
		default: break;
	}*/
	iEllapsedTime++;	

	switch (iState)
	{
		case EResolving:
			{
				if (iEllapsedTime > KHostResolveTimeout)
				{
					CloseL(_L8("Host lookup timeout"));
				}
			}
			break;

		case EConnecting:
			{
				if (iEllapsedTime > KConnectTimeout)
				{
					iHostCache->RemoveUltrapeer(iRemoteAddress);
					//iHostCache->DegradeUltrapeerL(iRemoteAddress);
					iHostCache->DegradeWorkingNodeL(iRemoteAddress);
					CloseL(_L8("Connection timeout"));
				}
			}
			break;

		case EHandshaking:
			{
				if (iEllapsedTime > KHandshakeTimeout)
				{
					iHostCache->RemoveUltrapeer(iRemoteAddress);
					//iHostCache->DegradeUltrapeerL(iRemoteAddress);
					iHostCache->DegradeWorkingNodeL(iRemoteAddress);
					CloseL(_L8("Handshake timeout"));
				}
			}
			break;

		case EGnutellaConnected:
			{
				//if we are in a connected state, and elapsed time is X mp, it means we are speaking to a good, quality
				//node, put it into the working nodes
				if ((iEllapsedTime == 5) && (!iAddressInWorkingNodes))
				{
					//LOG->WriteLineL(_L("Adding node to WorgingNodes"));
					iHostCache->SetWorkingNodeL(iRemoteAddress);
					iAddressInWorkingNodes = ETrue;
				}

				if (iEllapsedTime > KGeneralTimeout)
				{
					CloseL(_L8("Timeout (no data received)"));
				}
				else
				{
					//if (iPingInterval>-1) iPingInterval++;
					if (iEllapsedTime % KPingInterval == 0 || (IsSemPeer() && iHasProfile==0))
					{					
						PACKETPARSER->SendPingPacketL(*this);
					}

				}
			}
			break;

		default:;
			break;
	}
}


void CSANode::CloseL(const TDesC8& aReason, TBool /*aSendBye*/)
{
	CTR->ConnectionObserver()->SetStateL(iRemoteAddress,MSAConnectionObserver::EClose);
	
	Cancel();

	ChangeState(EClosing);
	CTR->ConnectionObserver()->RemoveAddressL(iRemoteAddress);

	
	if (aReason.Length() > 0)
	{
		HBufC* reason = HBufC::NewLC(aReason.Length() + 20);
		TPtr reasonPtr(reason->Des());		
		reasonPtr.Copy(aReason);
		reasonPtr.Insert(0, _L("Closing: "));
		LogL(*reason);
		CleanupStack::PopAndDestroy(); // reason
	}
	else
		LogL(_L("Closing"));
}


/*TInt CSANode::ParseInt(TLex8& aLex, TInt& aInt)
{
	TInt accumulator =  0;

	if (!aLex.Peek().IsDigit())
	{
		return KErrGeneral;
	}

	do
	{
		accumulator = (accumulator * 10) + ( (TInt)aLex.Get() - (TInt)'0' );
	}
	while (aLex.Peek().IsDigit());

	aInt = accumulator;

	return KErrNone;

}*/



void CSANode::RunL()
{
	//iLog->WriteLineL(_L("CSANode RunL"));

	switch (iState)
	{
	case EConnecting:
		{
			if (iStatus == KErrNone)
			// Connected successfully, sending handshake message
			{
				LogL(_L("Connected"));

				CTR->ConnectionObserver()->SetStateL(iRemoteAddress,MSAConnectionObserver::EHandshaking);
				ChangeState(EHandshaking);

				StartReceiving();

				//Handshake 1st step, sending initial data including capability headers
				SendL(KHandshakeGnutellaConnect);
				SendL(KHandshakeHeaderUserAgent);
				SendL(KHandshakeHeaderXUltrapeerFlase);
				SendL(KHandshakeHeaderXSemPeer);
				SendL(KHandshakeHeaderXQueryRouting);
				SendL(KHandshakeHeaderXMaxTTL);

				// sending X-Try-Ultrapeers
				if (iHostCache->UltrapeerCount() > 0)
				{
					SendL(KHandshakeFieldXTryUltrapeers);
					SendL(_L8(":"));

					TBuf<20> ipBuf;
					TBuf8<30> buf;
					TBuf8<10> portBuf;
					TInetAddr addr;
					iHostCache->ResetUltrapeers();

					for (TInt i=0; (i<5) && (i<iHostCache->UltrapeerCount()); i++)
					{
						buf.SetLength(0);

						if (i != 0 ) buf.Append(_L8(","));
						
						iHostCache->GetUltrapeer(addr);
						addr.Output(ipBuf);						
						buf.Append(ipBuf);
						buf.Append(_L8(":"));
						portBuf.Num(addr.Port());
						buf.Append(portBuf);

						SendL(buf);						
					}
					SendL(KHandshakeEnd);
				}

				SendL(KHandshakeEnd);

				//iLog->WriteLineL(_L("Handshake sent"));

				//send a starting ping packet
				//User::After(5000);
				//PACKETPARSER->SendPingPacketL(*this);
			}
			else
			// Connection failed
			{
				CTR->ConnectionObserver()->SetStateL(iRemoteAddress,MSAConnectionObserver::EError);
				
				LogL(_L("Connection failed: "), iStatus.Int());
				CloseL();

				// removing the bad address from the hostcache
				iHostCache->RemoveUltrapeer(iRemoteAddress);
				//iHostCache->DegradeUltrapeerL(iRemoteAddress);
				iHostCache->DegradeWorkingNodeL(iRemoteAddress);
			}				
		}
		break;

	case EResolving:
		{
			iResolver.Close();
			if (iStatus == KErrNone)
			// hostname resolved successfully
			{
				//iLog->WriteLineL(_L("Host resolved successfully"));	
			    TNameRecord nameRecord = (*iNameEntry)();

				TInetAddr addr = static_cast<TInetAddr>(nameRecord.iAddr);
				addr.SetPort(iRemoteAddress.Port());
				
				// checkinf if we are already conencted to the address
				if (iControl.IsConnectedTo(addr)) 
					CloseL();
				else			    			 
					ConnectL(addr);
			}
			else
			// lookup failed
			{
				iLog->WriteLineL(_L("Resolving failed"));
				ChangeState(ENotConnected);
			}

			delete iNameEntry;
			iNameEntry = 0;
		}
		break;

	default:;
	}
}

void CSANode::HandleWriteErrorL()
{
	CTR->ConnectionObserver()->SetStateL(iRemoteAddress,MSAConnectionObserver::EError);
	CloseL(_L8("socket write failed"));	
}


void CSANode::DoCancel()
{
	switch (iState)
	{
	case EConnecting:
		{
			Socket().CancelConnect();
		}
		break;

	case EResolving:
		{
			iResolver.Cancel();

			delete iNameEntry;
			iNameEntry = 0;
		}
		break;
	default:;
	}
}


void CSANode::HandleReadErrorL()
{
	CTR->ConnectionObserver()->SetStateL(iRemoteAddress,MSAConnectionObserver::EError);
	CloseL(_L8("reading socket failed (disconnected/owerflow)"));
	iHostCache->RemoveUltrapeer(iRemoteAddress);
	//iHostCache->DegradeUltrapeerL(iRemoteAddress);
}

void CSANode::LogL(const TDesC& aText)
{
#ifdef LOG_TO_FILE
	TBuf<20> address;
	iRemoteAddress.Output(address);
	LOG->WriteL(_L("[Node "));
	if (iRemoteAddress.Address() != 0)
		LOG->WriteL(address);
	LOG->WriteL(_L(":"));
	address.Num(iRemoteAddress.Port());
	LOG->WriteL(address);
	LOG->WriteL(_L("] "));
	LOG->WriteLineL(aText);
#endif
}

void CSANode::LogL(const TDesC& aText, TInt aNumber)
{
#ifdef LOG_TO_FILE
	TBuf<20> address;
	iRemoteAddress.Output(address);
	LOG->WriteL(_L("[Node "));
	if (iRemoteAddress.Address() != 0)
		LOG->WriteL(address);
	LOG->WriteL(_L(":"));
	address.Num(iRemoteAddress.Port());
	LOG->WriteL(address);
	LOG->WriteL(_L("] "));
	LOG->WriteL(aText);
	LOG->WriteLineL(aNumber);
#endif
}