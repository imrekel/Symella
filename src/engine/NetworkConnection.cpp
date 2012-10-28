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

#include "NetworkConnection.h"
#include "SALogger.h"
#include <CommDbConnPref.h>


CNetworkConnection::CNetworkConnection(RSocketServ& aSocketServer)
 : CActive(EPriorityStandard), iIapId(-1), iSocketServer(aSocketServer), iState(ENCStopped)
{ 	
}


void CNetworkConnection::ConstructL()
{
	CActiveScheduler::Add(this);
}


CNetworkConnection::~CNetworkConnection()
{
	Cancel();
	iConnection.Close();
}


void CNetworkConnection::StartL()
{
	if (iIapId <= 0) User::Leave(0);
	
	if ((iState == ENCStopped) && (!IsActive()))
	{		
		Close();
	
		TCommDbConnPref prefs;
		prefs.SetIapId(iIapId);
		prefs.SetDirection(ECommDbConnectionDirectionOutgoing);
		prefs.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
		prefs.SetBearerSet(KCommDbBearerUnknown);

		//Start Connection
		User::LeaveIfError(iConnection.Open(iSocketServer));

		iConnection.Start(prefs, iStatus);
		SetActive();
		
		LOG->WriteLineL(_L("[Network] Starting RConnection..."));
		
		/*User::WaitForRequest(status);
		
		LOG->WriteL(_L("RConnection Start status: "));
		LOG->WriteLineL(status.Int());
		
		if (status.Int() == KErrNone)
		{					
			iNetworkConnectionStarted = ETrue;
			return KErrNone;
		}*/
	}
}

		
void CNetworkConnection::Close()
{
	LOG->WriteLineL(_L("[Network] Network connection CLOSED"));
	Cancel();
	iConnection.Close();
	iState = ENCStopped;
}


void CNetworkConnection::DoCancel()
{
    iConnection.Close();
	iState = ENCStopped;
}


void CNetworkConnection::RunL()
{	
	if (iState != ENCStarted) // starting connection
	{
		if (iStatus.Int() == KErrNone)
	    {
			iState = ENCStarted;
			LOG->WriteLineL(_L("[Network] RConnection started succesfully!"));
			ScheduleProgressNotification();
			
			iNetworkConnectionObserver->NetworkConnectionStartedL(ETrue, *this);
	    }
	    else
	    {
	    	Close();
			LOG->WriteLineL(_L("[Network] Starting RConnection failed!"));
			
			iNetworkConnectionObserver->NetworkConnectionStartedL(EFalse, *this);
	    	
	    }		
	}
	else // progress notification
	{
		if (iStatus == KErrNone)
		{
			//if (iProgress.iStage==KLinkLayerOpen); // interface up
			if (iProgress().iStage == KConnectionUninitialised) // interface down
			{
				LOG->WriteLineL(_L("[Network] RConnection down!"));
				Close();
				
				iNetworkConnectionObserver->NetworkConnectionStartedL(EFalse, *this);				
			}
		}
		
		if (iState == ENCStarted)
			ScheduleProgressNotification();
	}       
}

void CNetworkConnection::ScheduleProgressNotification()
{
	if (IsActive()) return;

	// ...

	iConnection.ProgressNotification(iProgress, iStatus);
	SetActive(); 
}
