/*****************************************************************************
 * Copyright (C) 2006 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of SymTorrent
 *
 * SymTorrent is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SymTorrent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Symella; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#include "SocketReader.h"
#include "SALogger.h"
#include "SocketBase.h"
#include "ReadBuffer.h"

CSocketReader::CSocketReader(RSocket& aSocket, 
								 CSocketBase& aSocketBase,
								 CReadBuffer& aLongBuffer)
: CActive(EPriorityStandard),
  iSocket(aSocket), 
  iSocketBase(aSocketBase),
  iLongBuffer(aLongBuffer)
{
}


CSocketReader::~CSocketReader()
{
	Cancel();
}


void CSocketReader::ConstructL()
{
	CActiveScheduler::Add(this);

	iLog = LOG;
}


void CSocketReader::DoCancel()
{
    // Cancel asychronous read request
	iSocket.CancelRead();
}


void CSocketReader::RunL()
{
    // Active object request complete handler
    switch (iStatus.Int())
    {
        case KErrNone:
            // Character has been read from socket			
			iLongBuffer.AppendL(iBuffer);
			
		//	iLog->WriteL(_L8("in  ::::"));
		//	iLog->WriteLineL(iBuffer);

			
			iBuffer.SetLength(0);			
		    IssueRead(); // Immediately start another read		
			
            iSocketBase.OnReceiveL();
			
            break;
			
        default:
		//	iLog->WriteL(_L8("(reader) socket error "));
		//	iLog->WriteLineL(iStatus.Int());

			// closing connection
			iSocketBase.HandleReadErrorL();
            break;
    }	
}

void CSocketReader::IssueRead()
{
    // Initiate a new read from socket into iBuffer
//    __ASSERT_ALWAYS(!IsActive(), User::Panic(KPanicSocketReader, EActiveObjectIsActive));
	if (!IsActive())
	{
		iSocket.RecvOneOrMore(iBuffer, 0, iStatus, iLastRecvLength);
    	SetActive();
	}  
}

void CSocketReader::Start()
{
    // Initiate a new read from socket into iBuffer
    if (!IsActive())
    {
        IssueRead();
    }
}
