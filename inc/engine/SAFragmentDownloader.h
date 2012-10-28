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

#ifndef SAGFRAGMENTDOWNLOADER_H__
#define SAGFRAGMENTDOWNLOADER_H__

// INCLUDES
#include "SocketBase.h"

// FORWARD DECLARATIONS
class CSALogger;
class CSAControl;
class CSAHostCache;
class CSAFragment;

enum TCloseReason
{
	EConnectionFailed = 0,
	EDisconnectedWhileDownloading,
	ECancelDownload,
	EHandshakeFailed,
	EHandshakeFailedFull,
	EHandshakeFailedServiceUnavailable,
	EGeneral
};

/**
 * downloads a fragment of the selected file
 */
class CSAFragmentDownloader 
 : public CSocketBase,
   public MSocketOpenObserver
{
public:
	enum TNodeState
	{
		ENotConnected = 0,
		EConnecting,
		EGetHeader,
		EGetContent,
		EClosing
	};
	
	CSAFragmentDownloader(CSAFragment& aFragment, CNetworkManager* aNetMgr);

	~CSAFragmentDownloader();

	void ConstructL(const TUint aFileID,
					const TDesC8& aFileName,
					const TUint aRangeStart,
					const TUint aRangeEnd,
					const TUint aSelfIP);

	void ConnectL(const TInetAddr& aAddr);

	void ReconnectL();

	void CloseL(TCloseReason aReason = EGeneral);

	inline const TInetAddr& RemoteAddress();

	inline const TInetAddr& Address();

	inline TNodeState State();

	inline TInt ConnectionRetryCount();

	inline TCloseReason	CloseReason();

	inline TUint LastDataSize();

private: // from CSASocketBase

	void OnReceiveL();

	void HandleReadErrorL();

	void HandleWriteErrorL();
	
private: // from MSocketOpenObserver

	void SocketOpenedL(TBool aResult, RSocket& aSocket);

private: // from CActive

	void RunL();

	void DoCancel();

	/**
	 * Called by the timer (in CSAControl) in every second
	 */
	void OnTimerL();

	void ChangeState(TNodeState aState);

	void SendGetRequestL();

	TBool ParseHeaderL(const TPtrC8 aBuf);

	TInt ParseHeaderRow(const TDesC8& aHeaderRow);

	void SendL(const TDesC8& aDes);

private:

	TCloseReason			iCloseReason;

	TInt					iConnectionRetryCount;

	CSAFragment&			iFragment;

	TUint					iIncomingBytes;

	TUint					iSecsSinceStart;

	CSAControl*				iControl;

	CSALogger*				iLog;

	TUint					iEllapsedTime;

	TNodeState				iState;

	TInetAddr				iRemoteAddress;

	TBool					iAfterHeader;

	//variables for downloading
	TUint					iFileID;
	HBufC8*					iFileName;
	TUint					iRangeStart;
	TUint					iRangeEnd;
	TInetAddr				iSelfIP;

	TUint					iLastDataSize;

	//got variables from server
	TUint					iReceivedRangeStart;
	TUint					iReceivedRangeEnd;
	TUint					iReceivedContentLength;
	TUint					iReceivedRangeCurrentEnd;

	friend class CSAFragment; // for OnTimerL()
	friend class CSAFragment2; // for OnTimerL()

};

inline const TInetAddr& CSAFragmentDownloader::RemoteAddress() {
	return iRemoteAddress;
}

inline CSAFragmentDownloader::TNodeState CSAFragmentDownloader::State() {
	return iState;
}

inline TInt CSAFragmentDownloader::ConnectionRetryCount() {
	return iConnectionRetryCount;
}

inline TCloseReason	CSAFragmentDownloader::CloseReason() {
	return iCloseReason;
}

inline TUint CSAFragmentDownloader::LastDataSize()
{
	TUint result = iLastDataSize;
	iLastDataSize = 0;
	return result;
}

#endif