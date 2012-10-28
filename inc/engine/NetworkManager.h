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

#ifndef NETWORKMANAGER_H__
#define NETWORKMANAGER_H__

// FORWARD DECLARATIONS
class CSALogger;
class CZWidget;
class CSTTBaseData;
class CSTTConnectorProxy;
class CSTTConnectorListener;


// INCLUDES
//#include "STDefs.h"
#include <e32base.h>
#include <es_sock.h>
#include <in_sock.h>
#include <http\rhttpsession.h>
//#include "WriteBuffer.h"
#include "NetworkConnection.h"

/*#include "STTConnector.h"
#include "STTSocketCollector.h"
#include "STPreferences.h"*/

// #define NETWORKMGR TORRENTMGR->NetworkManager()

enum TSTProxyConnectionAtttemptResult
{
	EProxyConnectionSucceeded,
	EProxyConnectionFailed,
	EProxyConnectionPending
};


// FORWARD DECLARATIONS
class CNetworkConnection;

class MSocketOpenObserver {
public:
	virtual void SocketOpenedL(TBool aResult, RSocket& aSocket) = 0;
};

class MHostResolverOpenObserver {
public:
	virtual void HostResolverOpenedL(TBool aResult, RHostResolver& aHostResolver) = 0;
};

class MHTTPSessionOpenObserver {
public:
	virtual void HTTPSessionOpenedL(TBool aResult, RHTTPSession& aHTTPSession) = 0;
};

class MProxyConnectionObserver {
public:
	virtual void ReportProxyConnectionL(TBool aProxyConnectionSucceeded) = 0;
};


/**
 * Interface used to query the acces point id from the UI
 */
class MAccessPointSupplier
{
public:
	virtual TBool GetIapIdL(TInt32& aAccessPointId, TDes& aAccessPointName) = 0;
};


class MNetWorkManagerObserver
{
public:

	virtual void NetworkConnectionCanceledL() = 0;
};

	
/**
 * CNetworkManager
 */
class CNetworkManager : 	public CBase, 
						  	public MNetworkConnectionObserver
{		

	class TSocketWithObserver
	{
	public:
		TSocketWithObserver() 
		 : iSocket(NULL), iOpenObserver(NULL)
		{}
		
		RSocket* iSocket;
		MSocketOpenObserver* iOpenObserver;
	};
	
	class THostResolverWithObserver
	{
	public:
		THostResolverWithObserver() 
		 : iHostResolver(NULL), iOpenObserver(NULL)
		{}
		
		RHostResolver* iHostResolver;
		MHostResolverOpenObserver* iOpenObserver;
	};
	
	class THTTPSessionWithObserver
	{
	public:
		THTTPSessionWithObserver() 
		 : iSession(NULL), iOpenObserver(NULL)
		{}
		
		RHTTPSession* iSession;
		MHTTPSessionOpenObserver* iOpenObserver;
	};
	
public: // exported functions

public:

	static CNetworkManager* NewL();
	
	static CNetworkManager* NewLC();

	inline void SetAccesPointSupplier(MAccessPointSupplier* aIapSupplier);

	~CNetworkManager();
	
	inline void SetNetworkManagerObserver(MNetWorkManagerObserver* aNetMgrObserver);
	
	/**
	 * Will leave if Iap ID is unset (use SetNetworkConnectionId)
	 */
	void StartNetworkConnectionL();
	
	void CloseNetworkConnection();
		
	void OpenSocketL(RSocket& aSocket, MSocketOpenObserver* aObserver);
	
	void OpenHostResolverL(RHostResolver& aHostResolver, MHostResolverOpenObserver* aObserver);
	
	void OpenHTTPSessionL(RHTTPSession& aSession, MHTTPSessionOpenObserver* aObserver);
	
	inline TBool IsNetworkConnectionStarted() const;
		
	inline TBool IsIapSet() const;
	
	void Close(RHostResolver& aHostResolver);
	
	void Close(RSocket& aSocket);
	
	void Close(RHTTPSession& aSession);
			
	inline RSocketServ& SocketServ();
	
	TInt Address(TInetAddr& aAddress);
	
	void AddNetworkConnectionObserverL(MNetworkConnectionObserver* aObserver);
	
	void RemoveNetworkConnectionObserver(MNetworkConnectionObserver* aObserver);
	
	inline void SetNetworkConnectionId(TInt aIapId);
	
protected:

	CNetworkManager();

	void ConstructL();

private:

	void NetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection);

private:
			
	TBool GetIapIdL();
	
	void SetHTTPSessionInfoL(RHTTPSession& aSession);

	CSALogger*					iLog;

	TUint						iEllapsedTime;
	
	TUint						iIdleTime;
	
	CNetworkConnection*			iNetworkConnection;
	
	RSocketServ					iSocketServ;
	
	TBool						iConnectionActive;
	
	MAccessPointSupplier*		iAccessPointSupplier;
	
	RArray<TSocketWithObserver> 		iSockets;
	
	RArray<THostResolverWithObserver> 	iHostResolvers;
	
	RArray<THTTPSessionWithObserver> 	iHTTPSessions;
	
	RPointerArray<MProxyConnectionObserver> iProxyConnectionObservers;
	
	RPointerArray<MNetworkConnectionObserver> iNetworkConnectionObservers;
	
	MNetWorkManagerObserver* 	iNetMgrObserver;
	
	TInetAddr					iLocalAddress;
	
	TBool						iStartingNetworkConnection;
		
	TSTProxyConnectionAtttemptResult	iLastProxyConnectionAttemptResult;	
};	


inline TBool CNetworkManager::IsNetworkConnectionStarted() const {
	#if defined(__WINS__) && !defined(EKA2)
		return ETrue;
	#else
		return iNetworkConnection->IsStarted();
	#endif
}

inline void CNetworkManager::SetAccesPointSupplier(MAccessPointSupplier* aIapSupplier) {
	iAccessPointSupplier = aIapSupplier;
}

inline TBool CNetworkManager::IsIapSet() const {
	return iNetworkConnection->IsIapSet();
}

inline void CNetworkManager::SetNetworkConnectionId(TInt aIapId) {
	iNetworkConnection->SetIapId(aIapId);
}

inline void CNetworkManager::SetNetworkManagerObserver(MNetWorkManagerObserver* aNetMgrObserver) {
	iNetMgrObserver = aNetMgrObserver;
}


inline RSocketServ& CNetworkManager::SocketServ() {
	return iSocketServ;
}

#endif

// End of File
