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

#ifndef SANODE_H
#define SANODE_h

// INCLUDES
#include "SocketBase.h"

// FORWARD DECLARATIONS
class CSALogger;
class CSAControl;
class CSAHostCache;


/**
 * Encapsulates a connection with a gnutella node.
 */
class CSANode : public CSocketBase
{
public:

	enum TNodeState
	{
		ENotConnected = 0,
		EResolving,
		EConnecting,
		EHandshaking,
		EGnutellaConnected,
		EClosing
	};

	CSANode(CSAControl& aControl, CNetworkManager* aNetMgr);

	~CSANode();

	void ConstructL();

	void ConnectL(const TInetAddr& aAddr);

//	void ConnectL(const TDesC& aAddress, TUint aPort);

	void CloseL(const TDesC8& aReason = _L8(""), TBool aSendBye = EFalse);

	inline TBool IsUltrapeer();

	inline TBool IsSemPeer();

	inline const TInetAddr& RemoteAddress();

	inline TNodeState State();

	inline TBool AlreadySearched();

	inline void SetAlreadySearched(TBool);

	inline void SetDeletable();

	inline TBool IsDeletable();

	inline TInt HasProfile();
	
    inline CArrayFixFlat<TInt>* profile();
    
	inline void SetHasProfile(TInt);

	inline void SetSimilarity(TInt);
	
	inline TInt Similarity();

	inline void SetDocCount(TInt);
	
	inline TInt DocCount();

private: // from CSocketBase

	void OnReceiveL();

	void HandleReadErrorL();

	void HandleWriteErrorL();
	
private: // from MSocketOpenObserver
	
	void SocketOpenedL(TBool aResult, RSocket& aSocket);

private: // from CActive

	void RunL();

	void DoCancel();

private: // new

	/**
	 * Called by the timer (in CSAControl) in every second
	 */
	void OnTimerL();

	/**
	 * Parses and incoming handshake message.
	 */
	void ParseHandshakeL(const TDesC8& aBuf);

	static TInt ReadHeaderFieldName(TLex8& aLex, TPtrC8& aHeaderName);

	TInt ParseHeaderXUltrapeer(TLex8& aLex);

	TInt ParseHeaderXTryUltrapeersL(TLex8& aLex);

	TInt ParseHeaderSemPeerVersion(TLex8& aLex);

	TInt ParseHeaderRemoteIP(TLex8& aLex);
	
	void LogL(const TDesC& aText);
	
	void LogL(const TDesC& aText, TInt aNumber);

	//static TInt ParseInt(TLex8& aLex, TInt& aInt);

	void ChangeState(TNodeState aState);

private:

	TBool					iAddressInWorkingNodes;

	/**
	 * A reference to the main engine
	 */
	CSAControl&				iControl;
	CSAHostCache*			iHostCache;

	/**
	 * Used to track the time spent since the last event.
	 * Increased by OnTimerL in every second.
	 */
	TUint					iEllapsedTime;

	/**
	 * A reference to the log
	 */
	CSALogger*				iLog;

	/**
	 * DNS name resolver 
	 */
	RHostResolver               iResolver;

	/**
	 * The result from the name resolver.
	 *
	 * We store it on heap to be able to free the
	 * memory after resolving (it takes ~100 bytes)
	 */
	TNameEntry*                  iNameEntry;

	TNodeState				iState;

	TInt iHasProfile;
	
	CArrayFixFlat<TInt>* iProfile;
	
	//Count of music files stored
	TInt iDocCount;
	TInt iSimilarity;


	/*enum THandshakeState
	{
		EValidatingProtocol = 0,
		EAcceptingHeaders
		//EWaitingOk it's needed for inbound connections
	};

	THandshakeState iHandshakeState;*/

	enum TConnectionDirection
	{
		EInBound = 0,
		EOutbound
	};

	TConnectionDirection	iConnDir;

	TInetAddr				iRemoteAddress;

	/**
	 * true if the node is an ultrapeer
	 */
	TBool					iUltrapeer;

	TBool					iSemPeer;

	TBool					iAlreadySearched;

	TBool					iDeletable;


	friend class CSAControl; // for OnTimerL()
};


inline TBool CSANode::IsUltrapeer() {
	return iUltrapeer;
}

inline TBool CSANode::IsSemPeer() {
	return iSemPeer;
}


inline const TInetAddr& CSANode::RemoteAddress() {
	return iRemoteAddress;
}

inline CSANode::TNodeState CSANode::State() {
	return iState;
}

inline TBool CSANode::AlreadySearched() {
	return iAlreadySearched;
}

inline void CSANode::SetAlreadySearched(TBool state = ETrue) {
	iAlreadySearched = state;
}

inline void CSANode::SetDeletable() {
	iDeletable = ETrue;
}

inline TBool CSANode::IsDeletable() {
	return iDeletable;
}

inline TInt CSANode::HasProfile() {
	return iHasProfile;
}

inline CArrayFixFlat<TInt>* CSANode::profile() {
	return iProfile;
}

inline void CSANode::SetHasProfile(TInt aNewState) {
	iHasProfile = aNewState;
}

	inline void CSANode::SetSimilarity(TInt aSimilarity)
	{
		iSimilarity = aSimilarity;
	}
	
	inline TInt CSANode::Similarity() 
	{
		return iSimilarity;
	}

	inline void CSANode::SetDocCount(TInt aDocCount)
	{
		iDocCount = aDocCount;
	}
	
	inline TInt CSANode::DocCount() 
	{
		return iDocCount;
	}


#endif