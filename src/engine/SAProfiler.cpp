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

#include "SAProfiler.h"
#include "SALogger.h"
#include "WriteBuffer.h"
#include "SAControl.h"
#include "SAPreferences.h"
#include "ReadBuffer.h"
#include "SADownload.h"
#include "SADownloadManager.h"
#include "SA.pan"
#include "SocketReader.h"
#include "SADefs.h"
#include <aknnotewrappers.h>
#include <EscapeUtils.h>


const TUint KConnectTimeout			=	10;
const TUint KGeneralTimeout			=	15; // no data received
const TUint KHostResolveTimeout		=	15;

// must be given with IP address! (host resolver doesn't seem to work on emulator)
_LIT(KProfileAddress, "http://152.66.251.41/symella/update.php");

CSAProfiler::CSAProfiler(CSAControl& aControl, CNetworkManager* aNetMgr) 
 : CSocketBase(aNetMgr),
   iControl(aControl), 
   iState(ENotConnected)
{	
	iReceivedAddresses = 0;
}

CSAProfiler::~CSAProfiler()
{
	Cancel();

	delete iDownloadFileName;
	delete iNameEntry;	
	delete iHostName;
	delete iUrl;
}

void CSAProfiler::ConstructL()
{
	CSocketBase::ConstructL();

	iLog = CSALogger::InstanceL();

	iLog->WriteLineL(_L("Profiler initialized"));
	
	iType = EConnect;
	if(PREFERENCES->GetEnableConnectionProfiling())
		ConnectL(KProfileAddress);
}

void CSAProfiler::ConnectL(const TInetAddr& aAddr)
{
    __ASSERT_DEBUG( (iState == ENotConnected) || (iState == EResolving), 
		User::Panic(KSAPanicNode, ESAAlreadyConnected));
	
	__ASSERT_ALWAYS(!IsActive(), 
		User::Panic(KSAPanicNode, ESAActiveObjectIsActive));
		
	iRemoteAddress = aAddr;
	ChangeState(EConnecting);

	iLog->WriteLineL(_L("[Profiler] Opening socket"));
	OpenSocketL();	
}

void CSAProfiler::ConnectL(const TDesC& aUrl)
{
	if(iState==ENoConnection) return;
	iLog->WriteLineL(aUrl);
	if (aUrl.Length() < 8)
	{
		iLog->WriteLineL(_L("Wrong profiler address format!"));
		ChangeState(ENoConnection); 

		return;
	}
	//set remoteaddress port to http
	iRemoteAddress.SetPort(80);

	delete iUrl;
	iUrl = 0;
	iUrl = aUrl.AllocL();
	
	//get the hostname out of the adress
	TLex lex(*iUrl);

	lex.Inc(7);
	lex.Mark();
	//get the hostname out of the url
	while (lex.Peek() && (lex.Peek() != '/')) lex.Inc();
	if (lex.Peek() == 0)
	{
		iLog->WriteLineL(_L("Wrong profiler address format!"));
		return;
	}

	//found the end of it, get the hostname out
	delete iHostName;
	iHostName = 0;
	iHostName = lex.MarkedToken().AllocL();
	iLog->WriteLineL(*iHostName);
	
	if (iRemoteAddress.Input(*iHostName) == KErrNone)
	{
		ConnectL(iRemoteAddress);
	}
	else
	{
		CloseL();
	}

//	ChangeState(EResolving);

//	iLog->WriteLineL(_L("[Profiler] Opening host resolver"));
//	OpenHostResolverL();
}

void CSAProfiler::SocketOpenedL(TBool aResult, RSocket& /*aSocket*/)
{
	if (aResult)
	{
		iSocketOpened = ETrue;
		iLog->WriteLineL(_L("[Profiler] Connecting"));
		Socket().Connect(iRemoteAddress, iStatus);
		SetActive();
	}
	else
	{
		CloseL();
	}
}

void CSAProfiler::HostResolverOpenedL(TBool aResult, RHostResolver& /*aHostResolver*/)
{
	if (aResult)
	{
		delete iNameEntry;
		iNameEntry = 0;
		iNameEntry = new (ELeave) TNameEntry;
	
		iLog->WriteLineL(_L("[Profiler] Resolving url"));
		// DNS request for name resolution
		iResolver.GetByName(*iHostName, *iNameEntry, iStatus);
		SetActive();
	}
	else
	{
		CloseL();
	}
}

void CSAProfiler::ChangeState(TNodeState aState)
{
	iState = aState;
	iEllapsedTime = 0;
}

void CSAProfiler::OnReceiveL()
{
	if (iState == EGetHeader)
	{
		//parse only if there is \r\n\r\n
		_LIT8(KHeadersEnd, "\r\n\r\n");
		TInt offset;
		TPtr8 buf(iRecvBuffer->Ptr());			

		// check if the remote side has finished transfering
		if ( (offset = buf.Find(KHeadersEnd) ) != KErrNotFound )
		{
			offset += 4;
			//parse			
			if ( ParseHeaderL(buf.Left(offset)) )
			{
				ChangeState(EGetData);
				//remove it from the buffer
				iRecvBuffer->Delete(0, offset);
			}
			else
			{
				//no correct data, close connection
				CloseL();
			}
		}
	}
	if (iState == EGetData)
	{
		_LIT8(KEndOfLine,"\n");
		TInt offset;
		TPtr8 buf(iRecvBuffer->Ptr());

		iLog->WriteLineL(_L("Incoming Profiling data"));
		iLog->WriteLineL(buf);

		//check if there is a finished row in the buffer
		while ( (offset = buf.Find(KEndOfLine) ) != KErrNotFound )
		{
			offset++;
			//parse
			
			if(offset==10) 
			{
				TLex8 lex(buf.Left(offset));
				
				_LIT8(KTrue, "true");
				_LIT8(KFalse, "fals");
				lex.Mark();
				lex.Inc(4);

				if (lex.MarkedToken().Compare(KTrue) == 0)
				  PREFERENCES->SetEnableConnectionProfiling(ETrue);
				else
				if (lex.MarkedToken().Compare(KFalse) == 0)
				  PREFERENCES->SetEnableConnectionProfiling(EFalse);
				  
				//skip the pipe  
				lex.Inc();	  
				
				lex.Mark();
				lex.Inc(4);

				if (lex.MarkedToken().Compare(KTrue) == 0)
				  PREFERENCES->SetEnableProfiling(ETrue);
				else
				if (lex.MarkedToken().Compare(KFalse) == 0)
				  PREFERENCES->SetEnableProfiling(EFalse);
			}

			//remove it from the buffer
			iRecvBuffer->Delete(0, offset);
			buf.Set(iRecvBuffer->Ptr());
			ChangeState(EConnected);			
		}
	}
}

TBool CSAProfiler::ParseHeaderL(const TPtrC8 aBuf)
{
	iLog->WriteL(_L("Profiler ParseHeader: "));
	iLog->WriteLineL(aBuf);

	TLex8 lex(aBuf);
	//skip HTTP/1.1
	lex.Inc(9);
	//get the errorcode in
	lex.Mark();
	while (lex.Peek().IsDigit()) lex.Inc();

	_LIT8(K200, "200");
	_LIT8(K302, "302");	
	if ((lex.MarkedToken().Compare(K200) == 0) || (lex.MarkedToken().Compare(K302) == 0 ))
	{
		//iLog->WriteLineL(_L("Profiler GET ok"));
		return ETrue;
	}
	else
	{
		//iLog->WriteLineL(_L("{Profiler GET wrong"));
		return EFalse;
	}
}


void CSAProfiler::CloseL()
{
	// stop receiving

	Cancel();
	
	if (iSocketOpened)
	{
		CloseSocket();
		iSocketOpened = EFalse;
	}
	
//	CloseHostResolver();

	if(iState != ENoConnection)
		ChangeState(EClosing);
}

void CSAProfiler::RunL()
{
	switch (iState)
	{
	case EConnecting:
		{
			if (iStatus == KErrNone)
			// Connected successfully, sending handshake message
			{
				ChangeState(EGetHeader);

				StartReceiving();
				//1.1 request
				//TODO Here decide on what kind of request is sent out (connect, queryprofile, ...
				//move this to commandprofileL
				
				switch(iType) 
				{
					case EConnect:
						DoProfileConnectL();
						break;
					case EQuery:
						DoProfileQueryL();
						break;
					case EDownload:
						DoProfileDownloadL();
						break;
					case EDiskfull:
						DoProfileDiskfullL();
						break;
					case EGenre:
						DoProfileGenreL();
						break;						
					default:
						break;
				}
			}
			else
			// Connection failed
			{
				iLog->WriteL(_L("Profiler connection failed: "));
				iLog->WriteLineL(iStatus.Int());
				ChangeState(ENoConnection);
				CloseL();
			}
		}
		break;

	case EResolving:
		{
			iResolver.Close();
			if (iStatus == KErrNone)
			// hostname resolved successfully
			{				
			    TNameRecord nameRecord = (*iNameEntry)();

				TInetAddr addr = static_cast<TInetAddr>(nameRecord.iAddr);
				addr.SetPort(iRemoteAddress.Port());

				TBuf<30> ipBuf;
				addr.Output(ipBuf);
				iLog->WriteL(_L("Resolved: "));
				iLog->WriteLineL(ipBuf);
				
				// checkinf if we are already conencted to the address
				ConnectL(addr);
			}
			else
			// lookup failed
			{
				iLog->WriteLineL(_L("Resolving failed"));
				CloseL();
			}

			delete iNameEntry;
			iNameEntry = 0;
		}
		break;
	

	default:;
	}
}

void CSAProfiler::DoCancel()
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

void CSAProfiler::HandleReadErrorL()
{
	CloseL();
}

void CSAProfiler::HandleWriteErrorL()
{
	CloseL();	
}

void CSAProfiler::ProfileQueryL() 
{
	iType=EQuery;
	if((iState == ENotConnected || iState==EClosing ) && PREFERENCES->GetEnableProfiling()) 
		ConnectL(KProfileAddress);
}

void CSAProfiler::ProfileDownloadL(const TDesC8& aFileName, TInt aFileSize) 
{
	iType=EDownload;
	if((iState == ENotConnected || iState==EClosing ) && PREFERENCES->GetEnableProfiling())
	{
		delete iDownloadFileName;
		iDownloadFileName = 0;
		iDownloadFileName = aFileName.AllocL();
		iDownloadFileSize = aFileSize;
		
		ConnectL(KProfileAddress);
	}
		
}

void CSAProfiler::DoProfileDownloadL() 
{
	if( PREFERENCES->GetEnableConnectionProfiling() && PREFERENCES->GetEnableProfiling()) 
	{		
		HBufC8* url = HBufC8::NewLC(600);
		TPtr8 des = url->Des();

		des.Append(_L8("GET "));
		des.Append(*iUrl);				
		TBuf8<300> query8;
		TBuf<300> query;				
		_LIT(KFormat,"?userid=%d&command=download&sessionid=%d&string="); 				
		query.Format(KFormat, PREFERENCES->GetUserId(), CTR->SessionId());
		query8.Copy(query);
		des.Append(query8);
	
		des.Append(*(EscapeUtils::EscapeEncodeL(CTR->SearchString(), EscapeUtils::EEscapeUrlEncoded)));
		des.Append(_L8("&filename="));
		des.Append(*(EscapeUtils::EscapeEncodeL(*iDownloadFileName, EscapeUtils::EEscapeUrlEncoded)));
		des.Append(_L8("&filesize="));
		_LIT8(KFormat2, "%d");
		TBuf8<32> filesize;
		filesize.Format(KFormat2, iDownloadFileSize);
		des.Append(filesize);
		des.Append(_L8(" HTTP/1.1\r\n"));
		des.Append(_L8("Host: "));
		des.Append(*iHostName);
		des.Append(_L8("\r\nConnection: close\r\nUser-Agent: SymellaProfiler\r\n\r\n"));

		SendL(*url);
		CleanupStack::PopAndDestroy(); //url1-3
	}
}

void CSAProfiler::ProfileDiskfullL() 
{
	iType=EDiskfull;
	if((iState == ENotConnected || iState==EClosing ) && PREFERENCES->GetEnableProfiling()) 
		ConnectL(KProfileAddress);
}

void CSAProfiler::ProfileGenreL(TUint8 aGenre) 
{
	iType=EGenre;
	iGenre = aGenre;
	if((iState == ENotConnected || iState==EClosing ) && PREFERENCES->GetEnableProfiling()) 
		ConnectL(KProfileAddress);
}


void CSAProfiler::DoProfileDiskfullL() 
{
	if( PREFERENCES->GetEnableConnectionProfiling() && PREFERENCES->GetEnableProfiling()) 
	{	
		HBufC8* url = HBufC8::NewLC(600);
		TPtr8 des = url->Des();

		des.Append(_L8("GET "));
		des.Append(*iUrl);				
		TBuf8<300> query8;
		TBuf<300> query;				
		_LIT(KFormat,"?userid=%d&command=diskfull&sessionid=%d&string="); 				
		query.Format(KFormat, PREFERENCES->GetUserId(), CTR->SessionId());
		query8.Copy(query);
		des.Append(query8);
		des.Append(*(EscapeUtils::EscapeEncodeL(CTR->SearchString(), EscapeUtils::EEscapeUrlEncoded)));

		des.Append(_L8(" HTTP/1.1\r\n"));
		des.Append(_L8("Host: "));
		des.Append(*iHostName);
		des.Append(_L8("\r\nConnection: close\r\nUser-Agent: SymellaProfiler\r\n\r\n"));

		SendL(*url);
		CleanupStack::PopAndDestroy(); //url
	}
}

void CSAProfiler::DoProfileQueryL() 
{
	if( PREFERENCES->GetEnableConnectionProfiling() && PREFERENCES->GetEnableProfiling()) 
	{
		
		HBufC8* url = HBufC8::NewLC(600);
		TPtr8 des = url->Des();

		des.Append(_L8("GET "));
		des.Append(*iUrl);				
		TBuf8<300> query8;
		TBuf<300> query;				
		_LIT(KFormat,"?userid=%d&command=query&sessionid=%d&string="); 				
		query.Format(KFormat, PREFERENCES->GetUserId(), CTR->SessionId());
		query8.Copy(query);
		des.Append(query8);
		des.Append(*(EscapeUtils::EscapeEncodeL(CTR->SearchString(), EscapeUtils::EEscapeUrlEncoded)));

		des.Append(_L8(" HTTP/1.1\r\n"));
		des.Append(_L8("Host: "));
		des.Append(*iHostName);
		des.Append(_L8("\r\nConnection: close\r\nUser-Agent: SymellaProfiler\r\n\r\n"));

		SendL(*url);

		CleanupStack::PopAndDestroy(); //url
	}
}


void CSAProfiler::DoProfileConnectL() 
{

	if( PREFERENCES->GetEnableConnectionProfiling()) 
	{
		HBufC8* url = HBufC8::NewLC(400);
		TPtr8 des = url->Des();

		des.Append(_L8("GET "));
		des.Append(*iUrl);				
		TBuf8<300> query8;
		TBuf<300> query;
		
		// "sv" for Symella version
		// "pv" for platform version
		_LIT(KFormat,"?userid=%d&command=connect&sessionid=%d&pv=%d&sv=");
		
		// platform version
		TInt pv = 2;
		#ifdef EKA2
			pv = 3;
		#endif
						 				
		query.Format(KFormat, PREFERENCES->GetUserId(), CTR->SessionId(), pv);
				
		query.Append(SYMELLA_VERSION_LIT_URLENCODED);
		query8.Copy(query);
		des.Append(query8);

		des.Append(_L8(" HTTP/1.1\r\n"));
		des.Append(_L8("Host: "));
		des.Append(*iHostName);
		des.Append(_L8("\r\nConnection: close\r\nUser-Agent: SymellaProfiler\r\n\r\n"));

		SendL(*url);
		CleanupStack::PopAndDestroy(); //url				
	}
}

void CSAProfiler::DoProfileGenreL() 
{

	if( PREFERENCES->GetEnableConnectionProfiling() && PREFERENCES->GetEnableProfiling()) 
	{
		HBufC8* url = HBufC8::NewLC(600);
		TPtr8 des = url->Des();

		des.Append(_L8("GET "));
		des.Append(*iUrl);				
		TBuf8<300> query8;
		TBuf<300> query;				
		_LIT(KFormat,"?userid=%d&command=genre&sessionid=%d&genre=%d&string="); 				
		query.Format(KFormat, PREFERENCES->GetUserId(), CTR->SessionId(),iGenre);
		query8.Copy(query);
		des.Append(query8);
	
		des.Append(*(EscapeUtils::EscapeEncodeL(CTR->SearchString(), EscapeUtils::EEscapeUrlEncoded)));
		CSADownload* last = CTR->DlManager()->GetLastDownload();
		des.Append(_L8("&filename="));
		des.Append(*(EscapeUtils::EscapeEncodeL(last->Name(), EscapeUtils::EEscapeUrlEncoded)));
		des.Append(_L8("&filesize="));
		_LIT8(KFormat2, "%d");
		TBuf8<32> filesize;
		filesize.Format(KFormat2, last->Size());
		des.Append(filesize);
		des.Append(_L8(" HTTP/1.1\r\n"));
		des.Append(_L8("Host: "));
		des.Append(*iHostName);
		des.Append(_L8("\r\nConnection: close\r\nUser-Agent: SymellaProfiler\r\n\r\n"));

		SendL(*url);
		CleanupStack::PopAndDestroy(); //url
	}
}
