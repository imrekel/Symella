#include "NetworkManager.h"
#include "SALogger.h"
#include <CommDbConnPref.h>
#include <HttpStringConstants.h>
#include <in_sock.h>
#include <http\RHTTPSession.h>
#include <http\RHTTPHeaders.h>
#include <eikenv.h>


CNetworkManager* CNetworkManager::NewL()
{
	CNetworkManager* self = CNetworkManager::NewLC();
	CleanupStack::Pop();
	
	return self;
}
	
CNetworkManager* CNetworkManager::NewLC()
{
	CNetworkManager* self = new (ELeave) CNetworkManager();
	CleanupStack::PushL(self);
	self->ConstructL();
	
	return self;
}


CNetworkManager::CNetworkManager()
 : iLastProxyConnectionAttemptResult(EProxyConnectionPending)
{
}


void CNetworkManager::ConstructL()
{
	iLog = LOG;
	User::LeaveIfError(iSocketServ.Connect(128));	
	
	iNetworkConnection = new (ELeave) CNetworkConnection(iSocketServ);
	iNetworkConnection->ConstructL();
	iNetworkConnection->SetNetworkConnectionObserver(this);
}
 

CNetworkManager::~CNetworkManager()
{			
	iSockets.Reset();
	iHostResolvers.Reset();
	iHTTPSessions.Reset();
	iNetworkConnectionObservers.Reset();
		
	delete iNetworkConnection;
	
	iSocketServ.Close();
}

void CNetworkManager::StartNetworkConnectionL()
{
	LOG->WriteLineL(_L("[Network] Starting network connection"));
	#if !defined(__WINS__) || defined(EKA2)
	{
	
		if (!iStartingNetworkConnection)
		{
			iStartingNetworkConnection = ETrue;
			TBool iapSet = ETrue;
		
			if (!iNetworkConnection->IsIapSet())
			{
				LOG->WriteLineL(_L("[Network] Asking for acces point"));
				iapSet = GetIapIdL();
				
				if (!iapSet)
				{
					LOG->WriteLineL(_L("[Network] Acces point selection dialog canceled"));
					NetworkConnectionStartedL(EFalse, *iNetworkConnection);
					return;
				}				
			}
			else
				LOG->WriteLineL(_L("[Network] Access point is already set"));
				
			LOG->WriteL(_L("[Network] Access point id: "));
			LOG->WriteLineL(iNetworkConnection->IapId());
			
			if (iapSet)
			{				
				LOG->WriteL(_L("[Network] Starting network connection: "));
				LOG->WriteLineL(iNetworkConnection->IapId());
				iNetworkConnection->StartL();
			}
		}	
	}
	#endif
}


void CNetworkManager::CloseNetworkConnection()
{
	LOG->WriteL(_L("[Network] Closing network connection"));
	iNetworkConnection->Close();
}


void CNetworkManager::OpenHostResolverL(RHostResolver& aResolver, MHostResolverOpenObserver* aObserver)
{
	#if !defined(__WINS__) || defined(EKA2)
	{	
		if (!IsNetworkConnectionStarted())
		{
			THostResolverWithObserver resolver;
			resolver.iHostResolver = &aResolver;
			resolver.iOpenObserver = aObserver;
			
			User::LeaveIfError(iHostResolvers.Append(resolver));
			StartNetworkConnectionL();			
		}
		else
		{
			if (aResolver.Open(iSocketServ, KAfInet, 
				KProtocolInetTcp, iNetworkConnection->Connection()) == KErrNone)
				aObserver->HostResolverOpenedL(ETrue, aResolver);
			else
				aObserver->HostResolverOpenedL(EFalse, aResolver);
		}
		
		return;
	}
	#else
	{	
		if (aResolver.Open(iSocketServ, KAfInet, KProtocolInetTcp) == KErrNone)
			aObserver->HostResolverOpenedL(ETrue, aResolver);
		else
			aObserver->HostResolverOpenedL(EFalse, aResolver);
	}
	#endif
}


void CNetworkManager::OpenSocketL(RSocket& aSocket, MSocketOpenObserver* aObserver)
{
	#if !defined(__WINS__) || defined(EKA2)
	{	
		if (!IsNetworkConnectionStarted())
		{
			TSocketWithObserver socket;
			socket.iSocket = &aSocket;
			socket.iOpenObserver = aObserver;
			
			User::LeaveIfError(iSockets.Append(socket));
			StartNetworkConnectionL();			
		}
		else
		{
			if (aSocket.Open(iSocketServ, KAfInet, KSockStream, 
				KProtocolInetTcp, iNetworkConnection->Connection()) == KErrNone)
				aObserver->SocketOpenedL(ETrue, aSocket);
			else
				aObserver->SocketOpenedL(EFalse, aSocket);
		}
		
		return;
	}
	#else
	{
		if (aSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp) == KErrNone)
			aObserver->SocketOpenedL(ETrue, aSocket);
		else
			aObserver->SocketOpenedL(EFalse, aSocket);
	}
	#endif
}


void CNetworkManager::OpenHTTPSessionL(RHTTPSession& aSession, MHTTPSessionOpenObserver* aObserver)
{
	LOG->WriteLineL(_L("[NetworkManager] Opening HTTP session"));
	#if !defined(__WINS__) || defined(EKA2)
	{
		if (!IsNetworkConnectionStarted())
		{
			THTTPSessionWithObserver session;
			session.iSession = &aSession;
			session.iOpenObserver = aObserver;
			
			User::LeaveIfError(iHTTPSessions.Append(session));
			LOG->WriteLineL(_L("HTTP session registered"));
			StartNetworkConnectionL();			
		}
		else
		{
			aSession.OpenL();
			SetHTTPSessionInfoL(aSession);
			aObserver->HTTPSessionOpenedL(ETrue, aSession);
		}
			
	}
	#else
		aSession.OpenL();
		aObserver->HTTPSessionOpenedL(ETrue, aSession);
	#endif
}


void CNetworkManager::SetHTTPSessionInfoL(RHTTPSession& aSession)
{
	LOG->WriteLineL(_L("[NetworkManager] Setting HTTP connection info"));
	RStringPool strP = aSession.StringPool();
	RHTTPConnectionInfo connInfo = aSession.ConnectionInfo();

	connInfo.SetPropertyL(strP.StringF(HTTP::EHttpSocketServ, RHTTPSession::GetTable()),
		THTTPHdrVal(iSocketServ.Handle()) );

	TInt connPtr = REINTERPRET_CAST(TInt, &(iNetworkConnection->Connection()));

	connInfo.SetPropertyL ( strP.StringF(HTTP::EHttpSocketConnection, RHTTPSession::GetTable()),
		THTTPHdrVal(connPtr) );
}


TBool CNetworkManager::GetIapIdL()
{
	LOG->WriteLineL(_L("[NetworkManager] Getting Iap ID"));
	#if !defined(__WINS__) || defined(EKA2)
		if (!iNetworkConnection->IsIapSet())
		{	
			if (iAccessPointSupplier == 0)
				return EFalse;
			
			TInt32 iap;
			TBuf<150> iapName;
			if (iAccessPointSupplier->GetIapIdL(iap, iapName))
			{
				iNetworkConnection->SetIapId(iap);
				return ETrue;
			}
				
			//NetworkConnectionStartedL(EFalse, *iNetworkConnection);
				
										
			return EFalse;		
		}
	#endif
	
	return ETrue;	
}


void CNetworkManager::NetworkConnectionStartedL(TBool aResult, CNetworkConnection& /*aConnection*/)
{
	LOG->WriteLineL(_L("[NetworkManager] RConnection started"));
	{
		for (TInt i=0; i<iSockets.Count(); i++)
		{
			TBool result = aResult;			
			if (result)
			result = 
				((*iSockets[i].iSocket).Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp, iNetworkConnection->Connection()) == KErrNone);
				
			iSockets[i].iOpenObserver->SocketOpenedL(result, *iSockets[i].iSocket);
		}
			
	}
	{
		for (TInt i=0; i<iHostResolvers.Count(); i++)
		{
			TBool result = aResult;
			if (result)
			result = 
				((*iHostResolvers[i].iHostResolver).Open(iSocketServ, KAfInet, KProtocolInetUdp, iNetworkConnection->Connection()) == KErrNone);
		
			iHostResolvers[i].iOpenObserver->HostResolverOpenedL(result, *iHostResolvers[i].iHostResolver);	
		}
	}			
	{				
		for (TInt i=0; i<iHTTPSessions.Count(); i++)
		{
			TBool result = aResult;
			if (result)
			{
				(*iHTTPSessions[i].iSession).OpenL();
				SetHTTPSessionInfoL(*iHTTPSessions[i].iSession);
			}
			
			iHTTPSessions[i].iOpenObserver->HTTPSessionOpenedL(result, *iHTTPSessions[i].iSession);
		}
	}
	
	iSockets.Reset();
	iHostResolvers.Reset();
	iHTTPSessions.Reset();
		
	if ((!aResult) /*&& (PREFERENCES->AccessPointId() <= 0)*/)
		iNetworkConnection->SetIapId(-1);
	
	for (TInt i=0; i<iNetworkConnectionObservers.Count(); i++)
	{
		iNetworkConnectionObservers[i]->NetworkConnectionStartedL(aResult, *iNetworkConnection);
	}
	
	TInetAddr address;
	if (Address(address) == KErrNone)
	{
		TBuf<128> addrBuf;
		address.Output(addrBuf);
		LOG->WriteL(_L("[NetworkManager] Local address: "));
		LOG->WriteLineL(addrBuf);
	}
	
	iStartingNetworkConnection = EFalse;
}


void CNetworkManager::Close(RHostResolver& aHostResolver)
{
	TInt index = -1;
	for (TInt i=0; i<iHostResolvers.Count(); i++)
		if (iHostResolvers[i].iHostResolver == &aHostResolver)
			index = i;
	
	if (index >= 0)
		iHostResolvers.Remove(index);
	
	aHostResolver.Close();		
}


void CNetworkManager::Close(RSocket& aSocket)
{
	TInt index = -1;
	for (TInt i=0; i<iSockets.Count(); i++)
		if (iSockets[i].iSocket == &aSocket)
			index = i;
		
	if (index >= 0)
		iSockets.Remove(index);
		
	aSocket.Close();	
}

void CNetworkManager::Close(RHTTPSession& aSession)
{
	TInt index = -1;
	for (TInt i=0; i<iHTTPSessions.Count(); i++)
	if (iHTTPSessions[i].iSession == &aSession)
		index = i;
		
	if (index >= 0)
		iHTTPSessions.Remove(index);
	 
	aSession.Close();
}


TInt CNetworkManager::Address(TInetAddr& aAddress)
{
	
	if (iLocalAddress.Address() != 0)
	{
		aAddress = iLocalAddress;
		return KErrNone;
	}
			
	return KErrGeneral;
}

void CNetworkManager::AddNetworkConnectionObserverL(MNetworkConnectionObserver* aObserver)
{
	if (iNetworkConnectionObservers.Find(aObserver) < 0)
	{
		User::LeaveIfError(iNetworkConnectionObservers.Append(aObserver));
	}
}
	
void CNetworkManager::RemoveNetworkConnectionObserver(MNetworkConnectionObserver* aObserver)
{
	TInt index = iNetworkConnectionObservers.Find(aObserver);
	
	if (index >= 0)
	{
		iNetworkConnectionObservers.Remove(index);
	}
}