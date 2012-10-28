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

#ifndef SACONTROL_H
#define SACONTROL_H

// INCLUDES
#include <coemain.h>
#include "SAProfiler.h"
#include "SAGWebCache.h"
#include <in_sock.h>

// FORWARD DECLARATIONS
class CSALogger;
class CSANode;
class TInetAddr;
class CPeriodic;
class CSAHostCache;
class CSADownloadManager;
class CSAMessageCache;
class CNetworkManager;

// CONSTS
#define			MAXTTL    5; // needed for literal
const TInt		KMaxTTL = 5;

const TUint		KDefaultGnutellaPort = 6346;
const TUint		KDefaultListenerQueueSize = 10;

const TInt		KDefaultMaxConnectionCount = 8; // 8
const TInt		KGnutellaProtocolVersion = 6; // 0.6

const TInt		KDefaultMaxSemanticConnectionCount = 4;


const TInt		KMinLimitOfUnidentifieds = 4;

const TInt		KMaxSearchingNodes = 4;

const TUid KUidSAControl = { 0x10000EF2 };

#define CTR CSAControl::InstanceL()

//observer interface for connections and suchlike
class MSAConnectionObserver
{
public:
	enum TSAConnectionState
	{
		EIdle,
		EConnecting,
		EHandshaking,
		EConnected,
		EClose,
		EError,
		EDisconnected
	};
	//address handling functions
	IMPORT_C virtual void AddAddressL(const TInetAddr& aAddress, const TBool aIsSemPeer) = 0;
	IMPORT_C virtual void SetUltrapeerL(const TInetAddr& aAddress) = 0;
	IMPORT_C virtual void SetStateL(const TInetAddr& aAddress, TSAConnectionState aState) = 0;
	IMPORT_C virtual void RemoveAddressL(const TInetAddr& aAddress) = 0;

	//hostcache handling functions
	IMPORT_C virtual void SetUltrapeersCountL(TInt aValue) = 0;
	IMPORT_C virtual void SetLeafsCountL(TInt aValue) = 0;
	IMPORT_C virtual void SetUnidentifiedNodesCountL(TInt aValue) = 0;
	IMPORT_C virtual void SetWorkingNodesCountL(TInt aValue) = 0;
	IMPORT_C virtual void SetCountsL(TInt aUltrapeers,TInt aLeafs,TInt aUnidentifieds,TInt aWorkings) = 0;

	//webcache handling functions
	IMPORT_C virtual void InsertWebCacheL(TInt aWebcacheIndex, TSAWebCacheRequestType aType = EHostFileRequest) = 0;
	IMPORT_C virtual void ModifyWebCacheL(	TSAConnectionState aStatus ) = 0;
	IMPORT_C virtual void SetWebCacheAddressCountL(TInt aCount) = 0;
	IMPORT_C virtual void RemoveWebCacheL() = 0;
};


/**
 * The main engine.
 */
class CSAControl : public CCoeStatic, public MNetworkConnectionObserver
{
public:

	enum TSAControlState
	{
		EAutoConnect,
		EDisconnected
	};

	/**
	 * Creates a new gnutella node connection.
	 *
	 * If we are already connected to the node then returns false
	 */
	TBool ConnectNodeL(const TInetAddr& aAddr);

	
	//void ConnectNodeL(const TDesC& aAddress, TUint aPort);

	static CSAControl* InstanceL();

	~CSAControl();

	void ConstructL();

	void ConnectL();

	void DisconnectL();
	
	TInt CalculateSimilarity(CSANode* aNode);
	
	TInt CalculateSimilarity(CArrayFixFlat<TInt>* aProfile, TInt docCount);
	
	TInt MinimumSimilarity();


	/**
	 * @return reference to the host cache
	 */
	inline CSAHostCache& HostCache();

	/**
	 * @return reference to the message cache
	 */
	inline CSAMessageCache& MessageCache();

	/**
	 * @return pointer to the download manager
	 */
	inline CSADownloadManager* DlManager();

	/**
	 * @return the number of active connections
	 */
	inline TInt ConnectionCount();

	inline TInt SemanticConnectionCount();


	void SetConnectionObserverL(MSAConnectionObserver* aObserver);

	inline MSAConnectionObserver* ConnectionObserver();

	/**
	 * Returns true if we are already connected/connecting to the address
	 *
	 * @param aAddr the address we are looking for
	 */
	TBool IsConnectedTo(const TInetAddr& aAddr);

	void SearchL(TDesC8& aQuery);

	/**
	 * Puts the local ip address to the supplied parameter.
	 *
	 * @return KErrNone if the address is known
	 */
	TInt GetLocalAddress(TInetAddr& aAddr);

	/**
	 * Sets to local ip address. 
	 *
	 * Called by nodes during handshake if a "Remote-IP" header 
	 * is received.
	 */
	inline void SetLocalAddress(const TInetAddr& aAddr);

	inline TInt Genre();
	inline TInt SessionId();	
	inline CSAProfiler* Profiler();
    void SetGenre(TInt aGenre);

	inline TSAControlState State();

	HBufC* CreateConnectInfoL();

	void FireLessSimilarNode();
	
	inline const TDesC8& SearchString() const;
	
	inline CNetworkManager* NetMgr();
	
private: // from MNetworkConnectionObserver

	virtual void NetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection);
	
private:

	CSAControl();

private : // new

	/**
	 * Static wrapper for OnTimerL
	 */
	static TInt StaticOnTimerL(TAny* aObject);

	/**
	 * Called by timer periodically
	 */
	void OnTimerL();

	/**
	 * Creates a new node object and puts it into the
	 * node list. The new node is not connected to any address.
	 *
	 * @return a pointer to the newly created node
	 */
	CSANode* CreateNodeL();

private:


	TSAControlState			iState;
	TInt 					iSessionId;

	/**
	 * The time ellapsed since the last state change.
	 * It's increased by OnTimerL()
	 */
	TUint					iEllapsedTime;

	CSAHostCache*			iHostCache;
	
	CSAProfiler*				iProfiler;	

	CSAMessageCache*		iMessageCache;

	CSADownloadManager*		iDlManager;

	CSAGWebCache*			iWebCache;

	TInetAddr				iLocalAddress;

	/**
	 * A periodic timer. Calls OnTimerL() periodically (started during
	 * construction)
	 */
	CPeriodic*				iTimer;

	CSALogger*				iLog;

	RPointerArray<CSANode>	iNodes;

	TInt					iMaxConnectionCount;
	TInt					iMaxSemanticConnectionCount;

	MSAConnectionObserver*	iConnectionObserver;

	HBufC8*					iSearchString;

	TInt					iGenre; //semantic data
	
	CNetworkManager*		iNetMgr;
	
	TInt					iWebcacheReconnectTimer;
	
	TBool					iIssueWebcacheUrlRequest;
};


inline TInt CSAControl::SessionId() {
	return iSessionId;
}

inline TInt CSAControl::Genre() {
	return iGenre;
}



inline CSAHostCache& CSAControl::HostCache() {
	return *iHostCache;
}

inline TInt CSAControl::ConnectionCount() {
	return iNodes.Count();
}

inline TInt CSAControl::SemanticConnectionCount() {
	return (iNodes.Count() > iMaxConnectionCount-iMaxSemanticConnectionCount ? iNodes.Count()+iMaxSemanticConnectionCount-iMaxConnectionCount : 0);
}

inline CSADownloadManager* CSAControl::DlManager() {
	return iDlManager;
}

inline CSAProfiler* CSAControl::Profiler() {
	return iProfiler;
}

inline CSAMessageCache& CSAControl::MessageCache() {
	return *iMessageCache;
}

inline void CSAControl::SetLocalAddress(const TInetAddr& aAddr) {
	iLocalAddress = aAddr;
}

inline CSAControl::TSAControlState CSAControl::State() {
	return iState;
}

inline MSAConnectionObserver* CSAControl::ConnectionObserver() {
	return iConnectionObserver;
}

inline const TDesC8& CSAControl::SearchString() const {
	if (iSearchString)
		return *iSearchString;
	
	return KNullDesC8;
}

inline CNetworkManager* CSAControl::NetMgr() {
	return iNetMgr;
}


#endif
