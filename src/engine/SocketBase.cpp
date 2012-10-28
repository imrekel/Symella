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

#include "SocketBase.h"
#include "SocketReader.h"
#include "SocketWriter.h"
#include "SALogger.h"
#include "ReadBuffer.h"
#include "NetworkManager.h"
#include <EscapeUtils.h>
#include <utf.h>


CSocketBase::CSocketBase(CNetworkManager* aNetMgr) 
: CActive(EPriorityStandard), iNetMgr(aNetMgr)
{
}

void CSocketBase::ConstructL()
{
	CActiveScheduler::Add(this);
	
	iSocket = new (ELeave) RSocket;

	iRecvBuffer = new (ELeave) CReadBuffer;
	iRecvBuffer->ConstructL();

	iSocketReader = new (ELeave) CSocketReader(Socket(), *this, *iRecvBuffer);
	iSocketReader->ConstructL();

	iSocketWriter = new (ELeave) CSocketWriter(Socket(), *this);
	iSocketWriter->ConstructL();
}

void CSocketBase::OpenSocketL()
{
	iNetMgr->OpenSocketL(Socket(), this);
}

void CSocketBase::OpenHostResolverL()
{
	iNetMgr->OpenHostResolverL(iResolver, this);
}

void CSocketBase::Detach()
{
	iDetached = ETrue;
}


CSocketBase::~CSocketBase()
{	
	delete iSocketReader;
	delete iSocketWriter;

	delete iRecvBuffer;
	
	iNetMgr->Close(Socket());
	iNetMgr->Close(iResolver);
	
	delete iSocket;
}

void CSocketBase::CloseSocket()
{	
	iSocketReader->Cancel();
	iSocketWriter->Cancel();	
	Socket().CancelAll();
	iNetMgr->Close(Socket());
}

void CSocketBase::CloseHostResolver()
{
	iResolver.Cancel();
	iNetMgr->Close(iResolver);
}

void CSocketBase::StartReceiving()
{
	iSocketReader->Start();
}

void CSocketBase::SendUrlEncodedL(const TDesC16& aDes)
{
	HBufC8* utf = HBufC8::NewLC(aDes.Size());
	TPtr8 ptr (utf->Des());
	CnvUtfConverter::ConvertFromUnicodeToUtf8(ptr, aDes);
	SendUrlEncodedL(*utf);	
	CleanupStack::PopAndDestroy(); // utf
}


void CSocketBase::SendUrlEncodedL(const TDesC8& aDes) 
{
	HBufC8* encoded = EscapeUtils::EscapeEncodeL(aDes, EscapeUtils::EEscapeUrlEncoded);
	CleanupStack::PushL(encoded);
	iSocketWriter->WriteL(*encoded);
	CleanupStack::PopAndDestroy(); // encoded
}
