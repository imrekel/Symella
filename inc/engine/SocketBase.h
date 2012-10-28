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

#ifndef SOCKETBASE_H__
#define SOCKETBASE_H__

#include <in_sock.h>
#include "SocketWriter.h"
#include "NetworkManager.h"

class CSocketReader;
class CReadBuffer;

_LIT(KLocalHost, "127.0.0.1");

/**
 * CSocketBase
 */
class CSocketBase : public CActive,
					public MSocketOpenObserver,
					public MHostResolverOpenObserver
{
public:

	CSocketBase(CNetworkManager* aNetMgr);

	~CSocketBase();

	void ConstructL();

	/**
	 * Detaches (removes ownership) the RSocket handle from the object.
	 */
	void Detach();

	inline TBool IsDetached() const;

	void StartReceiving();

	inline void SendL(const TDesC8& aDes);
		
	inline void PutToSendBufferL(const TDesC8& aDes);
	
	inline void SendNow();
	
	void SendUrlEncodedL(const TDesC8& aDes);
	
	void SendUrlEncodedL(const TDesC16& aDes);	

	inline void SocketName(TName& aName);
	
	inline RSocket& Socket() const;
	
	inline RSocketServ& SocketServ();
	
private: // MHostResolverOpenObserver

	virtual void HostResolverOpenedL(TBool /*aResult*/, RHostResolver& /*aHostResolver*/)
	{		
	}

private: // MSocketOpenObserver
	
	virtual void SocketOpenedL(TBool /*aResult*/, RSocket& /*aSocket*/)
	{		
	}

protected:

	/**
	 * Called by CSocketReader when data has been read from
	 * the socket.
	 *
	 * @param aBuf The read data, supplied in a descriptor.
	 */
	virtual void OnReceiveL()
	{
	}

	/**
	 * Called by CSocketWriter when a send operation
	 * finished succesfully.
	 */
	virtual void OnSendCompleteL()
	{
	}

	virtual void HandleReadErrorL()
	{
	}

	virtual void HandleWriteErrorL()
	{
	}

	/**
	 * Opens the socket. The result is passed to SocketOpenedL (called
	 * by the framework).
	 */
	void OpenSocketL();

	void CloseSocket();
	
	void OpenHostResolverL();
	
	void CloseHostResolver();

	/**
	 *
	 * @discussion Tracks the state of this object through the connection process
	 * @value ENotConnected The initial (idle) state
	 * @value EConnecting A connect request is pending with the socket server
	 * @value EConnected A connection has been established
	 * @value ELookingUp A DNS lookup request is pending with the socket server
	 */
	enum TSocketState 
	{
		ENotConnected,
		EListening,
        EConnecting,
        EConnected,
		ELookingUp
	};

protected:

	RSocket*                  iSocket;

	CSocketReader*            iSocketReader;

	CSocketWriter*            iSocketWriter;

	CNetworkManager*			iNetMgr;
	
	RHostResolver               iResolver;

	CReadBuffer*				iRecvBuffer;

private: // Member variables

	TBool						iDetached;

	TSocketState				iSocketState;

	friend class CSocketReader;
	friend class CSocketWriter;

};


inline RSocket& CSocketBase::Socket() const {
	return *iSocket;
}

inline void CSocketBase::SocketName(TName& aName) {
	Socket().Name(aName);
}

inline void CSocketBase::SendL(const TDesC8& aDes) {
	iSocketWriter->WriteL(aDes);
}

inline TBool CSocketBase::IsDetached() const {
	return iDetached;
}

inline void CSocketBase::PutToSendBufferL(const TDesC8& aDes) {
	iSocketWriter->WriteWithoutSendingL(aDes);
}

inline void CSocketBase::SendNow() {
	iSocketWriter->SendNow();
}

inline RSocketServ& CSocketBase::SocketServ() {
	return iNetMgr->SocketServ();
}



#endif

