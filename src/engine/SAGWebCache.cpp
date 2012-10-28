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

// INCLUDE FILES
#include "SAGWebCache.h"
#include "SALogger.h"
#include "SAControl.h"
#include "SAHostCache.h"
#include "SAPreferences.h"
#include <uri8.h>
#include <http.h>
#include <chttpformencoder.h>
#include <HttpStringConstants.h>
#include <http\RHTTPTransaction.h>
#include <http\RHTTPSession.h>
#include <http\RHTTPHeaders.h>
#include <http\thttphdrfielditer.h>
#include <EscapeUtils.h>

// ================= MEMBER FUNCTIONS =======================


// Used user agent for requests
_LIT8(KUserAgent, "LimeWire/4.9.7");

// This client accepts all content types.
// (change to e.g. "text/plain" for plain text only)
_LIT8(KAccept, "*/*");

const TInt KConnectTimeout = 30;


CSAGWebCache::CSAGWebCache(CSAControl& aControl, TSAWebCacheRequestType aRequestType)
 : iControl(aControl), iRequestType(aRequestType)
{
}


CSAGWebCache::~CSAGWebCache()
{
	if (iRunning)
	{
		iTransaction.Close();
		iNetMgr->Close(iSession);		
	}
	
	delete iUri;	
	delete iReceiveBuffer;
	delete iFullWebcacheAddress;
}


void CSAGWebCache::ConstructL(const TDesC8& aUrl)
{	
	iLog = CSALogger::InstanceL();
	iNetMgr = CTR->NetMgr();
	iHostCache = &(iControl.HostCache());
	
	iFullWebcacheAddress = aUrl.AllocL();
	
	CBufFlat* uriBuf = CBufFlat::NewL(aUrl.Length() + 100);
	CleanupStack::PushL(uriBuf);
	
	uriBuf->InsertL(uriBuf->Size(), aUrl);
	uriBuf->InsertL(uriBuf->Size(), _L8("?"));
	
	if (iRequestType == EHostFileRequest)	
		uriBuf->InsertL(uriBuf->Size(), _L8("hostfile=1"));	
	else
		uriBuf->InsertL(uriBuf->Size(), _L8("urlfile=1"));
	
	iUri = uriBuf->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(); // uriBuf
}


void CSAGWebCache::SetHeaderL(RHTTPHeaders aHeaders, 
							 TInt aHdrField, 
							 const TDesC8& aHdrValue)
{
	RStringF valStr = iSession.StringPool().OpenFStringL(aHdrValue);
	CleanupClosePushL(valStr);	
	THTTPHdrVal val(valStr);
	aHeaders.SetFieldL(iSession.StringPool().StringF(aHdrField,
		RHTTPSession::GetTable()), val);
	CleanupStack::PopAndDestroy(); // valStr
}

void CSAGWebCache::StartTransactionL()
{
	if (!iRunning)
	{
		iNetMgr->Close(iSession);
	
		LOG->WriteLineL(_L("[GWebCache] Opening HTTP session"));
		iControl.ConnectionObserver()->ModifyWebCacheL(MSAConnectionObserver::EHandshaking);
		iNetMgr->OpenHTTPSessionL(iSession, this);	
		iRunning = ETrue;			
	}		
}

void CSAGWebCache::HTTPSessionOpenedL(TBool aResult, RHTTPSession& /*aHTTPSession*/)
{
	if (aResult)
	{
		LOG->WriteLineL(_L("[GWebCache] starting transaction"));
	
		// Parse string to URI (as defined in RFC2396)
		TUriParser8 uri;
		uri.Parse(*iUri);
		
		// Get request method string for HTTP GET
		RStringF method = iSession.StringPool().StringF(HTTP::EGET,
			RHTTPSession::GetTable());		

		// Open transaction with previous method and parsed uri. This class will
		// receive transaction events in MHFRunL and MHFRunError.
		iTransaction = iSession.OpenTransactionL(uri, *this, method);

		// Set headers for request; user agent and accepted content type
		RHTTPHeaders hdr = iTransaction.Request().GetHeaderCollection();
		SetHeaderL(hdr, HTTP::EUserAgent, KUserAgent);
		SetHeaderL(hdr, HTTP::EAccept, KAccept);

		iConnecting = ETrue;
		iTimeoutCounter = KConnectTimeout;
		// Submit the transaction. After this the framework will give transaction
		// events via MHFRunL and MHFRunError.
		LOG->WriteL(_L("[GWebCache] Starting transaction: GET "));
		LOG->WriteLineL(*iUri);
		iTransaction.SubmitL();
	}
	else
	{
		iRunning = EFalse;
		SetFailed();
		iControl.ConnectionObserver()->ModifyWebCacheL(MSAConnectionObserver::EError);
	}
}


void CSAGWebCache::SetFailed()
{
	iResult = EFailed;
	//iReconnectAfter = KDefaultDelayBeforeReconnectingDownload;	
}


void CSAGWebCache::Cancel()
{
	if(!iRunning) 
		return;

	// Close() also cancels transaction (Cancel() can also be used but 
	// resources allocated by transaction must be still freed with Close())
	iTransaction.Cancel();
	iTransaction.Close();

	// Not running anymore
	iRunning = EFalse;
}


void CSAGWebCache::MHFRunL(RHTTPTransaction aTransaction, 
						  			const THTTPEvent& aEvent)
{
	switch (aEvent.iStatus)
	{
		case THTTPEvent::EGotResponseHeaders:
		{
			iConnecting = EFalse;
			// HTTP response headers have been received. Use
			// aTransaction.Response() to get the response. However, it's not
			// necessary to do anything with the response when this event occurs.

			LOG->WriteLineL(_L("[GWebCache] Got HTTP headers"));
			// Get HTTP status code from header (e.g. 200)
			RHTTPResponse resp = aTransaction.Response();
			TInt status = resp.StatusCode();
			
			if (status != 200) // ERROR, hiba esetén mi legyen? 404-et lekezelni!
			{
				LOG->WriteL(_L("[GWebCache] Error, status = "));
				TBuf<20> numBuf;
				numBuf.Num(status);
				LOG->WriteLineL(numBuf);
				SetFailed();
				Cancel();
				break;
			}

			// Get status text (e.g. "OK")
		/*	TBuf<32> statusText;
			statusText.Copy(resp.StatusText().DesC());
			LOG->WriteLineL(statusText);*/
			
			RHTTPHeaders headers = 
				aTransaction.Response().GetHeaderCollection();		
			THTTPHdrFieldIter i =
				headers.Fields();
			for (i.First(); !(i.AtEnd()); ++i)
			{
				RStringF header = iSession.StringPool().StringF(i());
				
				/*if ((header.DesC() == _L8("Content-Type")) && (!iContentType))
				{
					LOG->WriteL(header.DesC());
					LOG->WriteL(_L(": "));
					THTTPHdrVal	val;
					headers.GetField(header, 0, val);
					RStringF value = val.StrF();
					LOG->WriteLineL(value.DesC());
					// iContentType = value.DesC().AllocL();
				}
				else*/
					// LOG->WriteLineL(header.DesC());
			}
			

	/*		TBuf<256> text;
			_LIT(KHeaderReceived, "Header received. Status: %d %S");
			text.Format(KHeaderReceived, status, &statusText);
			LOG->WriteLineL(text);*/
		}
		break;

		case THTTPEvent::EGotResponseBodyData:
		{	
			iConnecting = EFalse;		
			// Part (or all) of response's body data received. Use 
			// aTransaction.Response().Body()->GetNextDataPart() to get the actual
			// body data.						

			// Get the body data supplier
			MHTTPDataSupplier* body = aTransaction.Response().Body();
			TPtrC8 dataChunk;						

			// GetNextDataPart() returns ETrue, if the received part is the last 
			// one.
			TBool isLast = body->GetNextDataPart(dataChunk);
			
			//iDownloadedSize += dataChunk.Size();						
			
			//LOG->WriteL(_L8("*"));
			//LOG->WriteL(dataChunk);
			
			if (iReceiveBuffer)
			{
				HBufC8* temp = HBufC8::NewL(
					iReceiveBuffer->Length() + dataChunk.Length());
				TPtr8 tempPtr(temp->Des());
				tempPtr.Copy(*iReceiveBuffer);
				tempPtr.Append(dataChunk);
				
				delete iReceiveBuffer;
				iReceiveBuffer = temp;
			}
			else
				iReceiveBuffer = dataChunk.AllocL();

			// Always remember to release the body data.
			body->ReleaseData();
		
			// NOTE: isLast may not be ETrue even if last data part received.
			// (e.g. multipart response without content length field)
			// Use EResponseComplete to reliably determine when body is completely
			// received.
			if (isLast)
			{
				/*LOG->WriteLineL();
				_LIT(KBodyReceived,"Body received");
				LOG->WriteLineL(KBodyReceived);*/
				
				//CSTBencode* bencodedResponse = CSTBencode::ParseL(*iReceiveBuffer);	
				//LOG->WriteLineL(*iReceiveBuffer);
				//
				//if (bencodedResponse)
				//{
				//	CleanupStack::PushL(bencodedResponse);
				//	iTorrent.ProcessTrackerResponseL(bencodedResponse);
				//	CleanupStack::PopAndDestroy(); // bencodedResponse
				//}
			}
		}
		break;

		case THTTPEvent::EResponseComplete:
		{
			// Indicates that header & body of response is completely received.
			// No further action here needed.
			//_LIT(KTransactionComplete, "Transaction Complete");
			//LOG->WriteLineL(KTransactionComplete);
			//iResult = ESucceeded;
		}
		break;

		case THTTPEvent::ESucceeded:
		{
			iConnecting = EFalse;
			LOG->WriteLineL(_L("[GWebCache] HTTP transaction succeded"));
			iResult = ESucceeded;
			
			iControl.ConnectionObserver()->ModifyWebCacheL(MSAConnectionObserver::EConnected);

			_LIT8(KEndOfLine,"\n");
			TInt offset = 0;
			TInt index = 0;
			TPtr8 buf(iReceiveBuffer->Des());
			
			//LOG->WriteL(_L("[GWebCache] Received data:"));
			//LOG->WriteLineL(buf);
			
			TLex8 lex(iReceiveBuffer->Des());
			
			// number of hosts returned by the webcache
			TInt hostCount = 0;

			//check if there is a finished row in the buffer
			while ( lex.Peek() )
			{
				lex.Mark();
				TChar c = lex.Peek();
				
				while (c && (c != '\n'))
				{
					lex.Inc();
					c = lex.Peek();
				}
				
				if (c == '\n')
				{
					if (ParseHostFileL(lex.MarkedToken()))
						hostCount++;
					lex.Inc();
				}
				else
					break;									
			}
			
			if (iRequestType == EHostFileRequest)
				iControl.HostCache().SaveHostCacheL();
			
			aTransaction.Close();
			iRunning = EFalse;
			
			if (hostCount > 0)
				PREFERENCES->MoveWebCacheToTopL(*iFullWebcacheAddress);
		}
		break;

		case THTTPEvent::EFailed:
		{
			iConnecting = EFalse;
			LOG->WriteLineL(_L("[GWebCache] HTTP transaction failed"));
			SetFailed();
			
			iControl.ConnectionObserver()->ModifyWebCacheL(MSAConnectionObserver::EError);
			
			aTransaction.Close();
			iRunning = EFalse;
		}
		break;

		default:
			// There are more events in THTTPEvent, but they are not usually 
			// needed. However, event status smaller than zero should be handled 
			// correctly since it's error.
		{
			TBuf<64> text;
			if (aEvent.iStatus < 0)
			{
				LOG->WriteL(_L("[GWebCache] HTTP transaction failed, "));
				_LIT(KErrorStr, "Error: %d");
				text.Format(KErrorStr, aEvent.iStatus);
				LOG->WriteLineL(text);
				
				SetFailed();
				
				// Just close the transaction on errors
				aTransaction.Close();
				iRunning = EFalse;
			}
			else
			{
				// Other events are not errors (e.g. permanent and temporary
				// redirections)
				_LIT(KUnrecognisedEvent, "[GWebCache] Unrecognised event: %d");
				text.Format(KUnrecognisedEvent, aEvent.iStatus);
				LOG->WriteLineL(text);
			}		
		}
		break;
	}
}


TBool CSAGWebCache::ParseHostFileL(const TPtrC8 aBuf)
{	
	TBool correctIP;
	TInetAddr addr;
	TBuf8<30> ip;
	TBuf<30> ip2;
	TBuf8<8> port;
	TUint port2;
	TLex8 lex2;
	TInt separ;
	TInt portlength;
	
	if (iRequestType == EHostFileRequest)
	{
		if ((separ=aBuf.Find(_L8(":")))!=KErrNotFound)
		{
			correctIP = ETrue;
			//split the string to ip and port
			ip = aBuf.Left(separ);
			ip2.Copy(ip);
			//CyBerCi 061005 Because buf can be e.g. "ERROR: Client returned too early". We cannot assume the right part to be mx 8 bytes.
			portlength = aBuf.Length() - separ - 1;
			if(portlength>8) portlength = 8;
			
			port = aBuf.Right(portlength);
			lex2.Assign(port);
			if (lex2.Val(port2) != KErrNone) correctIP = EFalse;
			if (addr.Input(ip2) != KErrNone) correctIP = EFalse;
			if (correctIP != EFalse)
			{
				addr.SetPort(port2);
				//add the ip to the hostcache
				iHostCache->AddUnidentifiedL(addr);

				//modify got address count
				iReceivedAddresses++;
				iControl.ConnectionObserver()->SetWebCacheAddressCountL(iReceivedAddresses);

				iLog->WriteL(_L("[GWebCache] Got peer address: "));
				TBuf<30> tmp;
				addr.Output(tmp);
				iLog->WriteLineL(tmp);
				
				return ETrue;
			}
		}
	}
	else
	{
		TLex8 lex(aBuf);
		lex.Mark();
		
		while ((lex.Peek() != 0) && (lex.Peek() != '\r'))
			lex.Inc();
		
		if (lex.MarkedToken().Length() > 0)	
			PREFERENCES->AddWebCacheL(lex.MarkedToken(), PREFERENCES->WebCacheCount());
		
		iLog->WriteL(_L("[GWebCache] Got GWebCache url: "));
		iLog->WriteLineL(lex.MarkedToken());
		
		return ETrue;
	}
	
	return EFalse;
}


TInt CSAGWebCache::MHFRunError(TInt aError, 
							  RHTTPTransaction /*aTransaction*/, 
							  const THTTPEvent& /*aEvent*/)
{
	TBuf<64>	text;
	_LIT(KRunError, "[GWebCache] MHFRunError: %d");
	text.Format(KRunError, aError);
	LOG->WriteLineL(text);
	return KErrNone;
}

void CSAGWebCache::OnTimerL()
{
	if (iConnecting)
	{
		if (iTimeoutCounter > 0)
		{
			iTimeoutCounter--;
		}
		else
		{
			SetFailed();
			
			iControl.ConnectionObserver()->ModifyWebCacheL(MSAConnectionObserver::EError);
			
			iTransaction.Close();
			iRunning = EFalse;
		}
	}
}
