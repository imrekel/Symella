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

//#include <EscapeUtils.h>
#include "SAFragmentDownloader.h"
#include "SALogger.h"
#include "WriteBuffer.h"
#include "SAControl.h"
#include "ReadBuffer.h"
#include "SA.pan"
#include "SocketReader.h"
#include "SAFragment.h"
#include "SADownload.h"
#include "SAUrlEncode.h"

const TUint KConnectTimeout			=	10;
const TUint KHandshakeTimeout		=	20;
const TUint KGeneralTimeout			=	15; // no data received
const TUint KHostResolveTimeout		=	15;

CSAFragmentDownloader::CSAFragmentDownloader(CSAFragment& aFragment, CNetworkManager* aNetMgr) 
: CSocketBase(aNetMgr),
  iFragment(aFragment),
  iState(ENotConnected),
  iConnectionRetryCount(-1)
{
}

CSAFragmentDownloader::~CSAFragmentDownloader()
{
	Cancel();
	delete iFileName;
}

void CSAFragmentDownloader::ConstructL( const TUint aFileID,
										const TDesC8& aFileName,
										const TUint aRangeStart,
										const TUint aRangeEnd,
										const TUint aSelfIP )
{
	CSocketBase::ConstructL();
	iLog = CSALogger::InstanceL();
	iControl = CSAControl::InstanceL();

	iFileID = aFileID;
	iFileName = aFileName.AllocL();
	iRangeStart = aRangeStart;
	iRangeEnd = aRangeEnd;
	iSelfIP = aSelfIP;
}

void CSAFragmentDownloader::ConnectL(const TInetAddr& aAddr)
{
    //__ASSERT_DEBUG( (iState == ENotConnected) || (iState == EResolving), 
	//	User::Panic(KSAPanicNode, ESAAlreadyConnected));
	
	__ASSERT_ALWAYS(!IsActive(), 
		User::Panic(KSAPanicNode, ESAActiveObjectIsActive));  


	iRemoteAddress = aAddr;
	//iConnDir = EOutbound;
	ChangeState(EConnecting);

	LOG->WriteL(_L("To: "));	
	TBuf<30> buf;
	iRemoteAddress.Output(buf);
	LOG->WriteL(buf);
	LOG->WriteL(_L(":"));
	LOG->WriteLineL(iRemoteAddress.Port());	

	iSecsSinceStart=0;
	iIncomingBytes=0;
	iAfterHeader = EFalse;
	
	iConnectionRetryCount++;
	
	OpenSocketL();
}

void CSAFragmentDownloader::SocketOpenedL(TBool aResult, RSocket& /*aSocket*/)
{
	if (aResult)
	{
		Socket().Connect(iRemoteAddress, iStatus);
		SetActive();
	}
	else
	{
		CloseL(EConnectionFailed);
	}
}


void CSAFragmentDownloader::ChangeState(TNodeState aState)
{
	iState = aState;
	iEllapsedTime = 0;
}

void CSAFragmentDownloader::OnReceiveL()
{
	iIncomingBytes += iRecvBuffer->Ptr().Length();

	if (iState == EGetHeader)
	{
		//we have to know why we are refused
		//iLog->WriteL(_L("Incoming FD: ["));
		//iLog->WriteL(iRecvBuffer->Ptr());
		//iLog->WriteLineL(_L("]"));

		//parse only if there is \r\n\r\n
		_LIT8(KHeadersEnd, "\r\n\r\n");
		TInt offset;
		TPtr8 buf(iRecvBuffer->Ptr());			

		// check if the remote side has finished transfering
		if ( (offset = buf.Find(KHeadersEnd) ) != KErrNotFound )
		{			
			offset += 4;
			//parse
			TBool canGetData;

		/*	LOG->WriteL(_L("Incoming download header:["));
			LOG->WriteL(buf.Left(offset));
			LOG->WriteL(_L("]"));*/
			canGetData = ParseHeaderL(buf.Left(offset));

			if (canGetData)
			{
				ChangeState(EGetContent);
				iLog->WriteLineL(_L("Change to Get Content state"));
				//remove it from the buffer
				iRecvBuffer->Delete(0, offset);
			}
			else
			{
				iLog->WriteLineL(_L("Handshake failed in fragmentdownloader"));
				//no correct data, close connection
				CloseL(iCloseReason);
			}
		}		
	}
	else

	if (iState == EGetContent)
	{
		iEllapsedTime = 0;

		iLastDataSize += iRecvBuffer->Ptr().Length();
		iReceivedRangeCurrentEnd += iRecvBuffer->Ptr().Length();

		iFragment.WriteToFileL(iRecvBuffer->Ptr());
		iRecvBuffer->Delete(0, iRecvBuffer->Size());		
		
		iLog->WriteL(_L(" Pos:"));
		iLog->WriteLineL(iReceivedRangeCurrentEnd);
		
	}
}

TBool CSAFragmentDownloader::ParseHeaderL(const TPtrC8 aBuf)
{
	//iLog->WriteLineL(_L("Received header:"));
	//iLog->WriteLineL(aBuf);
	TLex8 lex(aBuf);
	//parse the first row
	//skip HTTP/1.1
	while (!lex.Get().IsSpace()) {}
	//lex.Inc(9);
	
	//get the errorcode in
	lex.Mark();
	while (lex.Peek().IsDigit()) lex.Inc();

	TBool connectAccepted = EFalse;

	//Error codes
	_LIT8(K200, "200"); //OK
	_LIT8(K206, "206");	//Partial content
	_LIT8(KBusy, "Busy"); //Busy
	_LIT8(KServiceUnavailable, "Service unavailable"); //service unavailable
	_LIT8(KFull, "Full"); //Full

	if ((lex.MarkedToken().Compare(K200) == 0)||(lex.MarkedToken().Compare(K206) == 0))
		connectAccepted = ETrue;
	if (!connectAccepted)
	{
		if (lex.MarkedToken().Compare(KBusy) == 0)
			iCloseReason = EHandshakeFailedFull;		
		else if (lex.MarkedToken().Compare(KServiceUnavailable) == 0)
			iCloseReason = EHandshakeFailedServiceUnavailable;
		else if (lex.MarkedToken().Compare(KFull) == 0)
			iCloseReason = EHandshakeFailedFull;
		else
			iCloseReason = EHandshakeFailed;
		return EFalse;
	}
	else
	{
		//proceed to the end of the first row
		while (lex.Get() != '\n')
		{			
		}

		//here are coming the header rows, until two \r\n
		//read the header rows until \r\n\r\n
		lex.Mark();
		TBool endOfHeader = EFalse;
		
		while (!endOfHeader)
		{
			TInt lineLength=0;
			while (lex.Get()!='\n') lineLength++;
			//when the linelength is just 1 from the last line, it means we reached a \r\n\r\n, the body comes
			if (lineLength==1)
				endOfHeader = ETrue;
			else
			{
				//we've got a header row, parse it
				ParseHeaderRow(lex.MarkedToken());
				//mark the end of it
				lex.Mark();
			}
		}
		return ETrue;
	}
}

TInt CSAFragmentDownloader::ParseHeaderRow(const TDesC8& aHeaderRow)
{
	_LIT8(KServer,"Server:");
	_LIT8(KContentType,"Content-Type:");
	_LIT8(KContentLength,"Content-Length:");
	_LIT8(KContentRange,"Content-Range: bytes");
	//_LIT8(KXAlt,"X-Alt: ");

	if (aHeaderRow.FindC(KServer)!=KErrNotFound)
		//serverheader - skipped
		return KErrNone;
	else if (aHeaderRow.FindC(KContentType)!=KErrNotFound)
		//contenttype header - skipped
		return KErrNone;
	else if (aHeaderRow.FindC(KContentLength)!=KErrNotFound)
		//contentlength header - skipped, or can be used for check
	{
		//TLex8 lex2;
		return KErrNone;
	}
	/*else if (aHeaderRow.FindC(KXAlt)!=KErrNotFound)
	{
		//parse the IP-s we got
		TLex8 lex2(aHeaderRow);

		//skip to the first space
		while (lex2.Get()!=' ') {}
		lex2.Inc();
		lex2.Mark();
		TBool exit = EFalse;
		while (!exit)
		{
			//read till the next comma or \r character
			while ((lex2.Peek()!='\r')&&(lex2.Peek()!=',')) {lex2.Inc();}
			if (lex2.Peek()=='\r') exit=ETrue;

			//parse the got IP
			TBuf8<40> ip = lex2.MarkedToken();
			TInetAddr addr;
			TUint port = 6346;
			TInt pos=ip.Length();
			//see if there is a port in it
			if ((pos = ip.FindC(_L8(":"))) != KErrNotFound)
			{
				TLex8 lex3(ip.Mid(pos+1));
				lex3.Val(port);
			}
			//create the ip
			TBuf<40> ip2;
			ip2.Copy(ip.Left(pos));
			addr.Input(ip2);
			addr.SetPort(port);

			//add this address to the addresslist of the download
			iFragment.ParentDownload().AddAddressL(addr.Address(),addr.Port(),_L8(""),-1);
		}
	}*/
	else if (aHeaderRow.FindC(KContentRange) != KErrNotFound)
	{
		//HBufC8* row = aHeaderRow.AllocL();

		/*TLex8 lex2(aHeaderRow);
		//skip until a : is found
		while (lex2.Get()!=':');
		//skip whitespace
		while (lex2.Peek().IsSpace()) {lex2.Inc();}
		//skip "bytes"
		while (lex2.Peek().IsAlpha()) {lex2.Inc();}
		//skip whitespace
		while (lex2.Peek().IsSpace()) {lex2.Inc();}

		//parse the first number, if there is any
		if (lex2.Peek()=='-')
		{
			iReceivedRangeStart = 0;
			lex2.Get(); // "-"
		}
		else
		{
			lex2.Mark();
			while (lex2.Peek().IsDigit()) {lex2.Inc();}
			if (lex2.Val(iReceivedRangeStart)!=KErrNone)
				iReceivedRangeStart = 0;
			lex2.Get(); // "-"
		}

		//parse the second number
		if ((lex2.Peek()=='\r')||(lex2.Peek()=='/'))
		{
			iReceivedRangeEnd = 0;
		}
		else
		{
			lex2.Mark();
			while (lex2.Peek().IsDigit()) {lex2.Inc();}
			if (lex2.Val(iReceivedRangeEnd)!=KErrNone)
				iReceivedRangeEnd = 0;
		}*/
		
		
		//parse range start and end
		/*TInt start=21;
		TInt end=aHeaderRow.Mid(10).Find(_L8("-"))+10;
		TInt per=aHeaderRow.Find(_L8("/"));

		if (end==start+1)
			iReceivedRangeStart=0;
		else
		{
			TPtrC8 buf(aHeaderRow.Mid(start,end-start));
			lex2.Assign(buf);
			if (lex2.Val(iReceivedRangeStart)!=KErrNone)
				iReceivedRangeStart = iRangeStart;
		}

		if (per==KErrNotFound)
			per=aHeaderRow.Length()-1;

		TPtrC8 buf(aHeaderRow.Mid(end+1,per-end-1));
		lex2.Assign(buf);
		if (lex2.Val(iReceivedRangeEnd)!=KErrNone)
			iReceivedRangeEnd = iRangeEnd;

		iReceivedRangeCurrentEnd = iReceivedRangeStart;

		iLog->WriteL(_L("Received content range:"));
		iLog->WriteL(iReceivedRangeStart);
		iLog->WriteL(_L("-"));
		iLog->WriteLineL(iReceivedRangeEnd);

		//delete row;*/

		return KErrNone;
	}
	else
	{
		return KErrGeneral;
	}
}

void CSAFragmentDownloader::OnTimerL()
{
	iEllapsedTime++;

	switch (iState)
	{
		case EConnecting:
			{
				if (iEllapsedTime > KConnectTimeout)
				{			
					CloseL(EConnectionFailed);
				}
			}
			break;

		case EGetHeader:
			{
				iSecsSinceStart++;

				if (iEllapsedTime > KHandshakeTimeout)
				{
					CloseL(EHandshakeFailed);
				}
			}
			break;

		case EGetContent:
			{
				iSecsSinceStart++;

				if (iEllapsedTime > KGeneralTimeout)
				{
					CloseL(EDisconnectedWhileDownloading);
				}
			}
			break;

		default:;
			break;
	}
}

void CSAFragmentDownloader::CloseL(TCloseReason aReason)
{
	iCloseReason = aReason;

	//log out what state are we
	/*if (iCloseReason == EConnectionFailed)
		iLog->WriteLineL(_L("Connection failed"));
	if (iCloseReason == EHandshakeFailed)
		iLog->WriteLineL(_L("Handshake failed"));
	if (iCloseReason == EDisconnectedWhileDownloading)
		iLog->WriteLineL(_L("Disconnected while downloading"));
	if (iCloseReason == ECancelDownload)
		iLog->WriteLineL(_L("Download cancelled"));
	if (iCloseReason == EHandshakeFailedFull)
		iLog->WriteLineL(_L("Servent was full"));
	if (iCloseReason == EHandshakeFailedBusy)
		iLog->WriteLineL(_L("Servent was busy"));
	if (iCloseReason == EHandshakeFailedServiceUnavailable)
		iLog->WriteLineL(_L("Servent: service unavailable"));
	if (iCloseReason == EGeneral)
		iLog->WriteLineL(_L("General connection error"));*/


	Cancel();

	ChangeState(EClosing);
}

void CSAFragmentDownloader::RunL()
{
	switch (iState)
	{
	case EConnecting:
		{
			if (iStatus == KErrNone)
			// Connected successfully, sending handshake message
			{
				LOG->WriteLineL(_L("Fragment connected!"));
				iIncomingBytes=0;
				iSecsSinceStart=0;
				iAfterHeader = EFalse;
				ChangeState(EGetHeader);

				StartReceiving();
				/*TBuf8<100> url;
				url.Append(_L8("GET "));
				url.Append(iUrl);
				url.Append(_L8(" HTTP/1.0\r\n\r\n"));
				SendL(url);*/
				//TODO: send the HTTP/1.1-compliant GET message
				SendGetRequestL();
			}
			else
			// Connection failed
			{
				iLog->WriteL(_L("FragmentDownloader connection failed: "));
				iLog->WriteLineL(iStatus.Int());
				CloseL(EConnectionFailed);
			}
		}
		break;

	default:;
	}
}

void CSAFragmentDownloader::DoCancel()
{
	switch (iState)
	{
	case EConnecting:
		{
			Socket().CancelConnect();
			iCloseReason = ECancelDownload;
		}
		break;
	default:;
	}
}

void CSAFragmentDownloader::ReconnectL()
{
	CloseL();
	
	CloseSocket();
	ChangeState(EConnecting);
	iSecsSinceStart=0;
	iIncomingBytes=0;
	iAfterHeader = EFalse;
	
	iConnectionRetryCount++;
	
	OpenSocketL();
}

void CSAFragmentDownloader::HandleReadErrorL()
{
	switch (iState)
	{
		case EConnecting:
			{				
				CloseL(EConnectionFailed);			
			}
			break;

		case EGetHeader:
			{
				CloseL(EHandshakeFailed);
			}
			break;

		case EGetContent:
			{
				CloseL(EDisconnectedWhileDownloading);
			}
			break;

		default:
			CloseL();
			break;
	}	
}


void CSAFragmentDownloader::HandleWriteErrorL()
{
	switch (iState)
	{
		case EConnecting:
			{				
				CloseL(EConnectionFailed);			
			}
			break;

		case EGetHeader:
			{
				CloseL(EHandshakeFailed);
			}
			break;

		case EGetContent:
			{
				CloseL(EDisconnectedWhileDownloading);
			}
			break;

		default:
			CloseL();
			break;
	}	
}

void CSAFragmentDownloader::SendGetRequestL()
{
	//iLog->WriteL(_L("Sent download Get request:["));
	HBufC8* buf = HBufC8::NewL(100+iFileName->Length());
	TPtr8 des = buf->Des();
	//TBuf8<400> buf;
	//send GET
	if (iFileName->Length()>0)
	{
		//HBufC8* encoded = EscapeUtils::EscapeEncodeL(*iFileName, EscapeUtils::EEscapeUrlEncoded);

		HBufC8* encoded = NSAUrlEncode::EncodeUrlL(*iFileName);
		_LIT8(KGet,"GET /get/%u/%S HTTP/1.1\r\n");
		//_LIT8(KGet,"GET /get/%u/%S HTTP/1.0\r\n");
		des.Format(KGet, iFileID, encoded);
		delete encoded;
		SendL(*buf);
	}
	else
	{
		//we have an alternate location, send another header
		_LIT8(KGet2,"GET /uri-res/N2R?urn:sha1:%S HTTP/1.1\r\n");
		TBuf8<32> hash;
		TSAHash hashh = iFragment.ParentDownload().Hash();
		hashh.Hash(hash);
		des.Format(KGet2, &hash);
		SendL(*buf);
	}

	//send user-agent
	_LIT8(KUserAgent,"User-Agent: BearShare 4.7.2.2\r\n");
	SendL(KUserAgent);

	//send host (HTTP 1.1 needs it)
	{
		TBuf<30> ip16;
		iRemoteAddress.Output(ip16);

		TBuf8<30> ip8;
		ip8.Copy(ip16);

		_LIT8(KHost,"Host: %S:%u\r\n");
		des.Format(KHost, &ip8, iRemoteAddress.Port());
		SendL(*buf);
	}

	//send connection close
	_LIT8(KConnection,"Connection: Close\r\n");
	SendL(KConnection);

	//if rangestart is -1, do not set range start
	if ((iRangeStart==-1)&&(iRangeEnd>-1))
	{
		//send range, and close the get header
		_LIT8(KRange,"Range: bytes=0-%u\r\n\r\n");
		des.Format(KRange, iRangeEnd);
		SendL(*buf);
	}
	else if ((iRangeStart > -1) && (iRangeEnd == -1))
	{
		//send range, and close the get header
		_LIT8(KRange,"Range: bytes=%u-\r\n\r\n");
		des.Format(KRange,iRangeStart);
		SendL(*buf);
	}
	else
	{
		//send range, and close the get header
		_LIT8(KRange,"Range: bytes=%u-%u\r\n\r\n");
		des.Format(KRange,iRangeStart,iRangeEnd);
		SendL(*buf);
	}
//	iLog->WriteLineL(_L("]"));
	delete buf;
}

void CSAFragmentDownloader::SendL(const TDesC8& aDes)
{
//	LOG->WriteL(aDes);
	CSocketBase::SendL(aDes);
}