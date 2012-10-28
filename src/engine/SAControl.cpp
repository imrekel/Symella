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

#include "SAControl.h"
#include "SANode.h"
#include "SALogger.h"
#include "SAMessageCache.h"
#include "SAPacketParser.h"
#include "SAPacket.h"
#include "SADownloadManager.h"
#include "SAGWebCache.h"
#include "SAHostCache.h"
#include "SAPreferences.h"
#include <aknnotewrappers.h> 
#include <e32math.h>

// CONSTS
const TInt		KHostCacheSaveInterval = 120; // in every two minutes
const TInt		KWebCacheSaveInterval = 180; // in every two minutes


CSAControl::CSAControl() 
	:	CCoeStatic(KUidSAControl), 
		iState(EDisconnected)
{
	iMaxConnectionCount = KDefaultMaxConnectionCount;	
	iMaxSemanticConnectionCount = KDefaultMaxSemanticConnectionCount;		
}


void CSAControl::ConstructL()
{
	iLog = LOG;

	iLog->WriteLineL(_L("*****************************"));
	iLog->WriteLineL(_L("[Symella/SemPeer] starting up"));
	
	iNetMgr = CNetworkManager::NewL();
	iNetMgr->AddNetworkConnectionObserverL(this);

	iHostCache = new (ELeave) CSAHostCache();
	iHostCache->ConstructL();

	iMessageCache = new (ELeave) CSAMessageCache();
	iMessageCache->ConstructL();

	iDlManager = new (ELeave) CSADownloadManager();
	iDlManager->ConstructL();
	
    iTimer = CPeriodic::NewL(0); // neutral priority
	// the timer ticks in every second
    iTimer->Start(1000000,1000000,
		TCallBack(StaticOnTimerL, this));

 	//Session uid generation
 	TTime now;
 	now.HomeTime();
   	TInt64 randSeed = now.Int64();
     	iSessionId = Math::Rand( randSeed );
	
	iLog->WriteLineL(_L("[Symella/SemPeer] initialized"));
}


CSAControl::~CSAControl()
{
	delete iTimer;

	iNodes.ResetAndDestroy();

	delete iSearchString;
	delete iHostCache;
	delete iMessageCache;
	delete iDlManager;
	delete iWebCache;
	delete iProfiler;	
	delete iNetMgr;
}


CSAControl* CSAControl::InstanceL()
{
	CSAControl* instance = static_cast<CSAControl*> 
		( CCoeEnv::Static( KUidSAControl ) );
	
	if (instance == 0)
	{
		instance = new (ELeave) CSAControl();

		CleanupStack::PushL( instance ); 
		instance->ConstructL(); 
		CleanupStack::Pop(); 
	}

	return instance;
}


void CSAControl::ConnectL()
{
//	iListener->StartListeningL(KDefaultGnutellaPort, KDefaultListenerQueueSize);
/*	TInetAddr addr;
	addr.Input(_L("192.168.0.100"));
	addr.SetPort(6346);
	ConnectNodeL(addr);*/

	if (iState == EDisconnected)
	{
		iState = EAutoConnect;

		iLog->WriteLineL(_L("[Control] Connecting"));
		iLog->WriteL(_L("[Control] WorkingNodes count: "));
		iLog->WriteLineL(iHostCache->WorkingNodesCount());
		
		delete iProfiler;
		iProfiler = 0;
			
		iProfiler = new (ELeave) CSAProfiler(*this, iNetMgr);
     	iProfiler->ConstructL();

		
         //TODO for testing comment out gwebcache (the following code forces the app to connect to
         // a web cache each time it starts connecting to the Gnutella network, for testing only!)
          /*
		if ((!iWebCache)&&(PREFERENCES->GetUseGWC()!=EFalse))
		{
			TBuf<100> webcacheAddress;
			PREFERENCES->GetNextGWebCacheL(webcacheAddress);

			iWebCache = new (ELeave) CSAGWebCache(*this);
			iWebCache->ConstructL();			
			iWebCache->ConnectL(webcacheAddress);
		} 
		else if ((iWebCache->State() == CSAGWebCache::ENotConnected)&&(PREFERENCES->GetUseGWC()!=EFalse))
		{
			TBuf<100> webcacheAddress;
			PREFERENCES->GetNextGWebCacheL(webcacheAddress);

			iWebCache = new (ELeave) CSAGWebCache(*this);
			iWebCache->ConstructL();			
			iWebCache->ConnectL(webcacheAddress);
		}  */
		
	}
}

void CSAControl::DisconnectL()
{
	if (iState != EDisconnected)
	{
		for (TInt i=0; i<iNodes.Count(); i++)
		{
			iNodes[i]->CloseL(_L8("forced disconnect"));
		}
		iState = EDisconnected;
		
		delete iSearchString;
		iSearchString = 0;
		
		if (iWebCache)
		{
			iConnectionObserver->RemoveWebCacheL();
			delete iWebCache;
			iWebCache = 0;
		}	
	
		if (iDlManager->SearchObserver())
			iDlManager->SearchObserver()->SearchStringChangedL(KNullDesC8);
	}
}


TBool CSAControl::ConnectNodeL(const TInetAddr& aAddr)
{
	if (IsConnectedTo(aAddr)) return EFalse;

	CSANode* node = CreateNodeL();
	node->ConnectL(aAddr);
	TUint port = aAddr.Port();

	return ETrue;
}


/*void CSAControl::ConnectNodeL(const TDesC& aAddress, TUint aPort)
{
	TInetAddr addr;
	if (addr.Input(aAddress) == KErrNone)
	{
		addr.SetPort(aPort);
		ConnectNodeL(addr);
	}
	else
	{
		CSANode* node = CreateNodeL();
		node->ConnectL(aAddress, aPort);
	}
}*/


TBool CSAControl::IsConnectedTo(const TInetAddr& aAddr)
{
	for (TInt i=0; i<iNodes.Count(); i++)
		if (iNodes[i]->RemoteAddress() == aAddr) return ETrue;

	return EFalse;
}


CSANode* CSAControl::CreateNodeL()
{
	CSANode* node = new (ELeave) CSANode(*this, NetMgr());
	CleanupStack::PushL(node);
	node->ConstructL();
	User::LeaveIfError(iNodes.Append(node));
	CleanupStack::Pop();

	return node;
}


TInt CSAControl::StaticOnTimerL(TAny* aObject)
{
	// cast, and call non-static function
	((CSAControl*)aObject)->OnTimerL();
	return 1;
}


void CSAControl::OnTimerL()
{
//	iLog->WriteLineL(_L("Control OnTimer"));

	// incresing counter
	iEllapsedTime++;
	
	if ((iEllapsedTime % KHostCacheSaveInterval) == 0)
	{
		HostCache().SaveHostCacheL();	
	}
	
	if ((iEllapsedTime % KHostCacheSaveInterval) == 0)
	{
		PREFERENCES->SaveWebCachesL();			
	}
		
	{
		// calling OnTimerL() fucntion of all nodes and deleting closed ones
		for (TInt i=0; i<iNodes.Count(); i++)
		{
			CSANode* node = iNodes[i];

			if (node->IsDeletable())
			{				
				iNodes.Remove(i);
				i--;
				iConnectionObserver->RemoveAddressL(node->RemoteAddress());
				delete node;
			}
			else
			{
				node->OnTimerL();
				
				if (node->State() == CSANode::EClosing)
				{					
				/*	iLog->WriteL(_L("Deleting node:"));
					TBuf<30> tmp;
					node->RemoteAddress().Output(tmp);
					iLog->WriteLineL(tmp);*/
					iHostCache->RemoveUnidentified(node->RemoteAddress());					
					iNodes.Remove(i);
					i--;
					iConnectionObserver->RemoveAddressL(node->RemoteAddress());
					delete node;
				}
			}
		}
		
		if (iWebcacheReconnectTimer > 0)
		{
			iWebcacheReconnectTimer--;
			
			if ((iWebcacheReconnectTimer == 0) && (iState == EAutoConnect) && iIssueWebcacheUrlRequest)			
			{
				iIssueWebcacheUrlRequest = EFalse;
				LOG->WriteLineL(_L("[Control] Requesting webcache urls"));
				iConnectionObserver->InsertWebCacheL(PREFERENCES->LastWebcacheIndex(), EUrlFileRequest);
				iWebCache = new (ELeave) CSAGWebCache(*this, EUrlFileRequest);
				iWebCache->ConstructL(PREFERENCES->GetLastWebCacheL());		
				iWebCache->StartTransactionL();
			}
		}


		if (iWebCache)
		{	
			iWebCache->OnTimerL();
								
			if ((iWebCache->Result() == CSAGWebCache::ESucceeded) || (iWebCache->Result() == CSAGWebCache::EFailed))
			{
				LOG->WriteLineL(_L("[Control] Webcache request complete"));
				TBool succeeded = EFalse;				
				if (iWebCache->Result() == CSAGWebCache::ESucceeded)
				{
					succeeded = ETrue;
				}
					
				
				TBool hostRequest = EFalse;
				if (iWebCache->RequestType() == EHostFileRequest)
					hostRequest = ETrue;
				
				if (PREFERENCES->WebCacheCount() <= 1)
					iWebcacheReconnectTimer = 30;
			
				iConnectionObserver->RemoveWebCacheL();
				delete iWebCache;
				iWebCache = 0;
				
				if (succeeded && hostRequest && (iState == EAutoConnect))
				{
					iWebcacheReconnectTimer = 30;
					iIssueWebcacheUrlRequest = ETrue;
					LOG->WriteLineL(_L("[Control] Request for webcache urls issued (connecting in 30 seconds)"));
					
				}
			}
			else
				if (iWebCache->IsConnecting())
					ConnectionObserver()->ModifyWebCacheL(MSAConnectionObserver::EConnecting);
		}
	}

	// TODO tesztelesig kivenni
	
	
//when we haven't got enough nodes in the unindentified, get more from a gwebcache
	if ((iHostCache->UltrapeerCount() < 10) && 
		(iHostCache->WorkingNodesCount() < 5) && 
		(iHostCache->UnidentifiedCount() < KMinLimitOfUnidentifieds)
		&& (iState == EAutoConnect) 
		&& (!iWebCache)
		&& (iWebcacheReconnectTimer <= 0))
	{	
		//create new gwebcache
		//add webcache row to the connections		
		TBuf8<160> webcacheAddress;
		TInt webcacheIndex = PREFERENCES->GetNextWebCacheL(webcacheAddress);
		iConnectionObserver->InsertWebCacheL(webcacheIndex);
		
		iWebCache = new (ELeave) CSAGWebCache(*this);
		iWebCache->ConstructL(webcacheAddress);		
		iWebCache->StartTransactionL();
	}
	
// idaig

	// opening new connections if we haven't got enough
	if ((iState == EAutoConnect) /*&& (!iWebCache)*/)
	{
		iHostCache->ResetUltrapeers();
		iHostCache->ResetWorkingNodes();
		iHostCache->ResetUnidentifieds();
		
		TInetAddr tryAddr;
		tryAddr.SetAddress(0);

		// if we haven't reached maximum connection count then open a new connection
		while (ConnectionCount() < /*1*/iMaxConnectionCount) //TODO teszt
		{
			// in every 4 seconds we try an unidentified node first
			if (iEllapsedTime % 4 == 0)
			{
				if (HostCache().GetUnidentified(tryAddr) != KErrNone)
					if (HostCache().GetUltrapeer(tryAddr) != KErrNone)
							HostCache().GetWorkingNode(tryAddr);					
			}
			else
			{
				if (HostCache().GetWorkingNode(tryAddr) != KErrNone)
					if (HostCache().GetUltrapeer(tryAddr) != KErrNone)
						HostCache().GetUnidentified(tryAddr);					
			}

			if (tryAddr.Address() != 0) 
			{
				//LOG->WriteLineL(tryAddr.Address());
				ConnectNodeL(tryAddr);
				tryAddr.SetAddress(0);
			}
			else
				break;
		}	
	}

	//count how many nodes have been used for searching
	TInt searchcount = 0;
	for (TInt i=0; i < iNodes.Count(); i++)
		if (iNodes[i]->AlreadySearched()) searchcount++;

	//if we have something to search, and we have some nodes that are already connected and can get search, 
	//and we have not filled in the searching nodes count, start new downloads
	if ((iSearchString)&&(iSearchString->Length() > 0)&&(searchcount < KMaxSearchingNodes))
	{
		for (TInt i=0; ((i < iNodes.Count()) && (searchcount < KMaxSearchingNodes)); i++)
		{
			CSANode* node = iNodes[i];

			if ( (!(node->AlreadySearched())) && (node->State() == CSANode::EGnutellaConnected) )
			{
				PACKETPARSER->SendQueryPacketL(*node, iMessageCache->SearchGuid(), 0, *iSearchString);
				//sign that we have searched on this node
				node->SetAlreadySearched();
				searchcount++;
			}		
		}
	}

	iDlManager->OnTimerL();
}

void CSAControl::SearchL(TDesC8& aQuery)
{
	// clearing the previous search results list
	iDlManager->ResetHitsList();

	//generate a guid and put it in the messagecache
	TGuid guid = PACKETPARSER->GenerateGuid();
	iMessageCache->AddGuidL(guid);
	//sign the Guid in the MessageCache as Search Guid
	iMessageCache->MarkAsSearch(guid);

	delete iSearchString;
	iSearchString = 0;
	iSearchString = aQuery.AllocL();
	
	if (iDlManager->SearchObserver())
		iDlManager->SearchObserver()->SearchStringChangedL(aQuery);
	
	//disconnect nodes that has been used for searching, so we can connect to it;
	for (TInt i=0; i<iNodes.Count(); i++)
	{
		CSANode* node = iNodes[i];
		if (node->State() == CSANode::EGnutellaConnected)
		{
			if (node->AlreadySearched())
			{
				//TODO: Removed by CyBerCi
				//PACKETPARSER->SendStopSearchPacketL(*node, guid);

				if(!node->IsSemPeer())
				    node->SetDeletable();
				node->SetAlreadySearched(EFalse);
			}
		}
	}
	
	Profiler()->ProfileQueryL();
}


TInt CSAControl::GetLocalAddress(TInetAddr& aAddr)
{
	if (iLocalAddress.Address() == 0)
		return KErrGeneral;

	aAddr = iLocalAddress;
	return KErrNone;
}


HBufC* CSAControl::CreateConnectInfoL()
{
	TBuf<400> buf;
	TBuf<40> ip;
	TBuf<20> tmp;

	buf.Format(_L("%d peer(s) connected:\n"),iNodes.Count());
	for (TInt i=0;i<iNodes.Count();i++)
	{
		if (iNodes[i]->IsUltrapeer())
			buf.Append(_L("(U)"));
		if (iNodes[i]->IsSemPeer())
			buf.Append(_L("(S)"));
		iNodes[i]->RemoteAddress().Output(ip);
		buf.Append(ip);
		tmp.Format(_L(":%u\n"),iNodes[i]->RemoteAddress().Port());
		buf.Append(tmp);
	}

	return buf.AllocL();
}


void CSAControl::SetConnectionObserverL(MSAConnectionObserver* aObserver)
{
	iConnectionObserver = aObserver;
	iHostCache->ShowCountsL();
}

TInt CSAControl::CalculateSimilarity(CSANode* aNode) {
    TInt similarity =  100*aNode->profile()->At(iGenre) /aNode->DocCount();
    
				_LIT(KFormat, "similarity: %d");
				TBuf<50> text;
				text.Format(KFormat, similarity);
				CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
				note->ExecuteLD(text);
				User::After(5000);    
				
				return similarity;
  }

TInt CSAControl::CalculateSimilarity(CArrayFixFlat<TInt>* aProfile, TInt aDocCount) {
    TInt similarity =  100*aProfile->At(iGenre) /aDocCount;
    
				_LIT(KFormat, "similarity: %d");
				TBuf<50> text;
				text.Format(KFormat, similarity);
				CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
				note->ExecuteLD(text);
				User::After(5000);    
				
				return similarity;
  }


void CSAControl::SetGenre(TInt aGenre) {
	iGenre = aGenre;
	
	//create new similarities
	for (TInt i=0;i<iNodes.Count();i++)
	{
		if (iNodes[i]->IsSemPeer())
			iNodes[i]->SetSimilarity(CalculateSimilarity(iNodes[i] ) );
	}
	
}

//returns the minimum similarity value among the node's (semantic) connections.
TInt CSAControl::MinimumSimilarity() {
		TInt minSimilarity = 101;
		for (TInt i=0; i<iNodes.Count(); i++)
		{
			if((i>=iMaxConnectionCount-iMaxSemanticConnectionCount) && iNodes[i]->Similarity() < minSimilarity)
				minSimilarity = iNodes[i]->Similarity();
		}
		if (minSimilarity == 101)
			minSimilarity = -1;
		return minSimilarity;
		
}

void CSAControl::FireLessSimilarNode() 
{
		TInt pos=-1;
		TInt minSimilarity = 101;
		for (TInt i=0; i<iNodes.Count(); i++)
		{
			if((i>=iMaxConnectionCount-iMaxSemanticConnectionCount) && iNodes[i]->Similarity() < minSimilarity) 
			{
				
				minSimilarity = iNodes[i]->Similarity();
				pos = i;
			}
		}
	if(pos != -1)
		iNodes[pos]->CloseL(_L8("Too small similarity"), 1);
	
}


void CSAControl::NetworkConnectionStartedL(TBool aResult, CNetworkConnection& /*aConnection*/)
{
	if (!aResult)
		DisconnectL();
}
