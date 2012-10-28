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

#ifndef SAGWEBCACHE_H__
#define SAGWEBCACHE_H__

// INCLUDES
#include <http\mhttpdatasupplier.h>
#include <http\mhttptransactioncallback.h>
#include <http\mhttpauthenticationcallback.h>
#include "SADefs.h"
#include "NetworkManager.h"

// FORWARD DECLARATIONS
class RHTTPSession;
class RHTTPTransaction;
class CSALogger;
class CSAControl;
class CSAHostCache;

enum TSAWebCacheRequestType
{
	EHostFileRequest = 0,
	EUrlFileRequest
};


/**
 * CSAGWebCache
 */
class CSAGWebCache : 	public CBase, 
						public MHTTPTransactionCallback, 
						public MHTTPSessionOpenObserver
{
public:
	
	enum TDownloadResult
	{
		EPending = 0,
		EFailed,
		ESucceeded		
	};

	CSAGWebCache(CSAControl& aControl, TSAWebCacheRequestType aRequestType = EHostFileRequest);

	void ConstructL(const TDesC8& aUrl);
	
	~CSAGWebCache();
	
	void StartTransactionL();			

	void Cancel();
	
	inline TBool IsRunning() const;
	
	inline TDownloadResult Result() const;
	
	inline TSAWebCacheRequestType RequestType() const;
	
	inline const TDesC8& Url() const;
	
	inline TBool IsConnecting() const;
	
	void OnTimerL();
	
private: // from MHTTPSessionOpenObserver

	virtual void HTTPSessionOpenedL(TBool aResult, RHTTPSession& aHTTPSession);
					
private:

	void SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, 
					const TDesC8& aHdrValue);
	
	void SetFailed();
	
	/**
	 * return true if a webcache address is parsed successfully
	 */
	TBool ParseHostFileL(const TPtrC8 aBuf);
	
private: // from MHTTPSessionEventCallback

	void MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
	
	TInt MHFRunError(	TInt aError, 
						RHTTPTransaction aTransaction, 
						const THTTPEvent& aEvent);
						
private:

	CSAControl&				iControl;
	
	CSAHostCache*			iHostCache;

	/**
	 * A reference to the log
	 */
	CSALogger*				iLog;

	/**
	 * Delay before reconnecting
	 */
	TUint					iReconnectAfter;
	
	/**
	 * The number of connection retries
	 */
	TUint					iRetries;	
		
	RHTTPSession			iSession;
	
	RHTTPTransaction		iTransaction;
	
	TBool					iRunning;	// ETrue, if HTTP transaction is running
	
	TDownloadResult			iResult;
	
	HBufC8*					iUri;
	
	HBufC8*					iReceiveBuffer;
	
	TSAWebCacheRequestType  iRequestType;
	
	TInt					iReceivedAddresses;
	
	CNetworkManager*		iNetMgr;
	
	HBufC8*					iFullWebcacheAddress;
	
	TInt					iTimeoutCounter;
	
	/**
	 * The network connection is started and the webcache is connecting
	 */
	TBool					iConnecting;

};	


inline TBool CSAGWebCache::IsRunning() const { 
	return iRunning; 
}

inline CSAGWebCache::TDownloadResult CSAGWebCache::Result() const {
	return iResult;
}

inline TSAWebCacheRequestType CSAGWebCache::RequestType() const {
	return iRequestType;
}

const TDesC8& CSAGWebCache::Url() const {
	return *iUri;
}

inline TBool CSAGWebCache::IsConnecting() const {
	return iConnecting;
}

#endif
