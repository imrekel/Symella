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

#ifndef SAPROFILER_H
#define SAPROFILER_H

// INCLUDES
#include "SocketBase.h"

// FORWARD DECLARATIONS
class CSALogger;
class CSAControl;
class CSAHostCache;


class CSAProfiler : public CSocketBase, 
					public MHostResolverOpenObserver,
					public MSocketOpenObserver
{
public:

	enum TNodeState
	{
		ENotConnected = 0,
		EResolving,
		EConnecting,
		EGetHeader,
		EGetData,
		EClosing,
		EConnected,
		ENoConnection
	};

	enum TProfilerType
	{
		EConnect = 0,
		EQuery,
		EDownload,
		EDiskfull,
		EGenre
	};

	CSAProfiler(CSAControl& aControl, CNetworkManager* aNetMgr);

	~CSAProfiler();

	void ConstructL();

	void ConnectL(const TInetAddr& aAddr);

	void ConnectL(const TDesC& aUrl);

	void CloseL();

	inline const TInetAddr& RemoteAddress();

	inline TNodeState State();
	
	void ProfileQueryL();
	void DoProfileQueryL();
	void DoProfileConnectL();
	void ProfileDownloadL(const TDesC8& aFileName, TInt aFileSize);
	void DoProfileDownloadL();
	void ProfileDiskfullL();
	void DoProfileDiskfullL();
	void ProfileGenreL(TUint8 aGenre);
	void DoProfileGenreL();

private: // from CSASocketBase

	void OnReceiveL();

	void HandleReadErrorL();
	
	void HandleWriteErrorL();
	
private: // MHostResolverOpenObserver

	void HostResolverOpenedL(TBool aResult, RHostResolver& aHostResolver);


private: // MSocketOpenObserver
	
	void SocketOpenedL(TBool aResult, RSocket& aSocket);

private: // from CActive

	void RunL();

	void DoCancel();

private:

	void ChangeState(TNodeState aState); 

	void ParseHostFileL(const TPtrC8 aBuf);

	TBool ParseHeaderL(const TPtrC8 aBuf);

private:
	/**
	 * A reference to the main engine
	 */
	CSAControl&				iControl;

	TUint					iEllapsedTime;

	/**
	 * A reference to the log
	 */
	CSALogger*				iLog;

	/**
	 * DNS name resolver 
	 */
	RHostResolver			iResolver;

	/**
	 * The result from the name resolver.
	 *
	 * We store it on heap to be able to free the
	 * memory after resolving (it takes ~100 bytes)
	 */
	TNameEntry*             iNameEntry;

	TNodeState				iState;
	
	TProfilerType			iType;

	HBufC*					iUrl;

	TInetAddr				iRemoteAddress;

	HBufC*					iHostName;

	TInt					iReceivedAddresses;
	
	TUint8					iGenre;
	
	HBufC8*					iDownloadFileName;
	TInt 					iDownloadFileSize;
	
	TBool					iSocketOpened;
};

inline const TInetAddr& CSAProfiler::RemoteAddress() {
	return iRemoteAddress;
}

inline CSAProfiler::TNodeState CSAProfiler::State() {
	return iState;
}


#endif
