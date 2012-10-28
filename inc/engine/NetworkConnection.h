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

/**
 * ============================================================================
 *  Name     : CNetworkConnection from NetworkConnection.h
 *  Part of  : SymTorrent
 *  Created  : 11.01.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef NETWORKCONNECTION_H__
#define NETWORKCONNECTION_H__

// INCLUDES
#include <Es_sock.h>

//FORWARD DECLARATIONS
class CNetworkConnection;

/**
 *  MNetworkConnectionObserver
 */
class MNetworkConnectionObserver
{
public:
	virtual void NetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection) = 0;
};

	
/**
 *  CNetworkConnection
 */
class CNetworkConnection : public CActive
{
public:

	enum TNetworkConnectionState
	{
		ENCStopped = 0,
		ENCStarting,
		ENCStarted	
	};
	
	CNetworkConnection(RSocketServ& aSocketServer);
	
	void ConstructL();
	
	~CNetworkConnection();
		
	/**
	 * Will leave if Iap ID is unset.
	 */
	void StartL();
	
	void Stop();
	
	void Close();
	
	inline void SetIapId(TInt aIapId);
	
	inline TInt IapId() const;
	
	inline TNetworkConnectionState State() const;
	
	inline TBool IsStarted() const;
	
	inline RConnection& Connection();
	
	inline TBool IsIapSet() const;
	
	inline void SetNetworkConnectionObserver(MNetworkConnectionObserver* aNetworkConnectionObserver);
		
private:

	void ScheduleProgressNotification();

private:  // from CActive		

	void RunL();

	void DoCancel();						

private:

	TInt			iIapId;

	RConnection		iConnection;
	
	RSocketServ& 	iSocketServer;
	
	TNetworkConnectionState iState;
	
	TNifProgressBuf iProgress;
	
	MNetworkConnectionObserver* iNetworkConnectionObserver;	
};


inline CNetworkConnection::TNetworkConnectionState CNetworkConnection::State() const {
	return iState;		
}

inline TBool CNetworkConnection::IsStarted() const {
	return (iState == ENCStarted);		
}

inline RConnection& CNetworkConnection::Connection() {
	return iConnection;		
}

inline TBool CNetworkConnection::IsIapSet() const {
	return (iIapId > 0);
}

inline TInt CNetworkConnection::IapId() const {
	return iIapId;
}

inline void CNetworkConnection::SetIapId(TInt aIapId) {
	iIapId = aIapId;
}

inline void CNetworkConnection::SetNetworkConnectionObserver(MNetworkConnectionObserver* aNetworkConnectionObserver) {
	iNetworkConnectionObserver = aNetworkConnectionObserver;	
}


#endif

// End of File
