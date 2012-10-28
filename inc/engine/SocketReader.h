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

#ifndef SOCKETREADER_H
#define SOCKETREADER_H

#include <in_sock.h>

class CSocketBase;
class CReadBuffer;
class CSALogger;

#define READBUFFERSIZE 10240

/**
 * Active object responsible for handling asynchronous socket read operations.
 */
class CSocketReader : public CActive
{
public:

	CSocketReader(RSocket& aSocket, 
					CSocketBase& aSocketBase, 
					CReadBuffer& aLongBuffer);

	void ConstructL();

	~CSocketReader();

	/**
	 * Activates the object (starts reading from the socket)
	 */
	void Start();

protected:

	void IssueRead();

protected: // from CActive

	void RunL();

	void DoCancel();

private:

	CSALogger*				iLog;
	
	RSocket&                iSocket;

	CSocketBase&			iSocketBase;

	CReadBuffer&			iLongBuffer;

	TBuf8<READBUFFERSIZE>	iBuffer;

	TSockXfrLength			iLastRecvLength;
};

#endif
