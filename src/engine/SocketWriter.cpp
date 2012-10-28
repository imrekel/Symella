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

#include "SocketWriter.h"
#include "SocketBase.h"
#include "SALogger.h"
#include "WriteBuffer.h"

CSocketWriter::CSocketWriter(RSocket& aSocket, CSocketBase& aSocketBase)
: CActive(EPriorityStandard),
  iSocket(aSocket), 
  iSocketBase(aSocketBase)
{
}


CSocketWriter::~CSocketWriter()
{
	Cancel();
	delete iLongBuffer;
}


void CSocketWriter::ConstructL()
{
	CActiveScheduler::Add(this);

	iLongBuffer = new (ELeave) CWriteBuffer;
	iLongBuffer->ConstructL();
}


void CSocketWriter::DoCancel()
{
    // Cancel asychronous write request
	iSocket.CancelWrite();
}


void CSocketWriter::RunL()
{
    // Active object request complete handler
    switch (iStatus.Int())
    {
        case KErrNone:
            // write completed succesfully, delegating event to
			// SocketBase
		//	iSocketBase.OnSendCompleteL();
			
		//	LOG->WriteL(_L8("out ::::"));
		//	LOG->WriteLineL(iShortBuffer);
			            
			if (iLongBuffer->Size() > 0) IssueWrite();
            break;
        default:
			iSocketBase.HandleWriteErrorL();
            break;
    }
}


void CSocketWriter::WriteL(const TDesC8& aBuf)
{
	iLongBuffer->AppendL(aBuf);
	if (!IsActive()) IssueWrite();
}


void CSocketWriter::WriteWithoutSendingL(const TDesC8& aBuf)
{
	iLongBuffer->AppendL(aBuf);
}


void CSocketWriter::SendNow()
{
	if (!IsActive()) IssueWrite();
}


void CSocketWriter::IssueWrite()
{
   // __ASSERT_ALWAYS(!IsActive(), User::Panic(KPanicSocketWriter, EActiveObjectIsActive));
	iLongBuffer->Read(iShortBuffer);
    iSocket.Write(iShortBuffer, iStatus);
  //  LOG->WriteL(_L("send::::"));
  //  LOG->WriteLineL(iShortBuffer);
    SetActive();
}
