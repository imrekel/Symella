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

#ifndef SOCKETWRITER_H
#define SOCKETWRITER_H

// INCLUDES
#include "WriteBuffer.h"
#include <in_sock.h>

// FORWARD DECLARATIONS
class CSocketBase;

const TUint KWriteBufferSize = 2048;

/**
 * Active object for handling write operations to a socket
 */
class CSocketWriter : public CActive
{
public:

	CSocketWriter(RSocket& aSocket, CSocketBase& aSocketBase);

	void ConstructL();

	~CSocketWriter();

	void WriteL(const TDesC8& aBuf);
	
	void WriteWithoutSendingL(const TDesC8& aBuf);
	
	void SendNow();
	
	inline void ResetLongBuffer();

protected:

	void IssueWrite();

protected: // from CActive

	void RunL();

	void DoCancel();

private:

    RSocket&					iSocket;

	CSocketBase&				iSocketBase;

	CWriteBuffer*				iLongBuffer;

	TBuf8<KWriteBufferSize>	iShortBuffer;

	friend class CSocketBase;
};

inline void CSocketWriter::ResetLongBuffer() {
	iLongBuffer->Reset();	
}

#endif
