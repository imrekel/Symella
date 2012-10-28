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

/*
* ============================================================================
*  Name     : CSymellaAppUi from SymellaAppUi.cpp
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include "SymellaAppUi.h"
#include "SymellaConsoleView.h"
#include "SymellaSearchView.h"
#include "SymellaTransfersView.h"
#include "SymellaConnectionsView.h"
#include "SymellaSettingsView.h"
#include "SymellaWebCacheView.h"
#include "SADefs.h"
#include <Symella.rsg>
#include <aknnotewrappers.h>
#include "symella.hrh"
#include "SALogger.h"
#include "SAControl.h"
#include "SANode.h"
#include "SAHostCache.h"
#include "SADownloadManager.h"
#include "SAHash.h"
#include "SAProfiler.h"
#include "SAUrlEncode.h"
#include <aknlistquerydialog.h> 
#include "AccessPointSettingItem.h"

#include <apgtask.h>
#include <avkon.hrh>
#include <eikmenup.h>
#include <aknmessagequerydialog.h>

_LIT(KAbout, "\nhttp://symella.aut.bme.hu\nAuthors:\nImre Kelenyi\nBertalan Forstner\nBalazs Molnar\n\nDeveloped in BUTE Department of Automation and Applied Informatics\nThis is free software, under the GNU General Public License.\n");

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CSymellaAppUi::ConstructL()
// ?implementation_description
// ----------------------------------------------------------
//
void CSymellaAppUi::ConstructL()
{
  BaseConstructL( EAknEnableSkin );

    // Show tabs for main views from resources
    CEikStatusPane* sp = StatusPane();

    // Fetch pointer to the default navi pane control
    iNaviPane = (CAknNavigationControlContainer*)sp->ControlL( 
        TUid::Uid(EEikStatusPaneUidNavi));

    // Tabgroup has been read from resource and it were pushed to the navi pane. 
    // Get pointer to the navigation decorator with the ResourceDecorator() function. 
    // Application owns the decorator and it has responsibility to delete the object.
    iDecoratedTabGroup = iNaviPane->ResourceDecorator();
    if (iDecoratedTabGroup)
        {
        iTabGroup = (CAknTabGroup*) iDecoratedTabGroup->DecoratedControl();
        }

	CSymellaConnectionsView* connectionsView = new (ELeave) CSymellaConnectionsView;

    CleanupStack::PushL( connectionsView );
    connectionsView->ConstructL();
    AddViewL( connectionsView );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // connectionsView

    CSymellaSearchView* searchView = new (ELeave) CSymellaSearchView;

    CleanupStack::PushL( searchView );
    searchView->ConstructL();
    AddViewL( searchView );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // searchView

    CSymellaTransfersView* transfersView = new (ELeave) CSymellaTransfersView;

    CleanupStack::PushL( transfersView );
    transfersView->ConstructL();
    AddViewL( transfersView );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // transfersView

#if defined(CONSOLE)
    CSymellaConsoleView* consoleView = new (ELeave) CSymellaConsoleView;

    CleanupStack::PushL( consoleView );
    consoleView->ConstructL();
    AddViewL( consoleView );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // consoleView
#endif

	SetDefaultViewL(*connectionsView);

	CSymellaSettingsView* settingsView = new (ELeave) CSymellaSettingsView;
	CleanupStack::PushL( settingsView );
	settingsView->ConstructL();
	AddViewL(settingsView);
	CleanupStack::Pop();    // settingsView

#if defined(CONSOLE)
	LOG->SetObserver(consoleView);
#endif

	CSymellaWebCacheView* webcacheView = new (ELeave) CSymellaWebCacheView;
    CleanupStack::PushL( webcacheView );
    webcacheView->ConstructL(this);
    AddViewL( webcacheView );      
    CleanupStack::Pop();

	CTR->NetMgr()->SetAccesPointSupplier(this);
	CTR->DlManager()->SetSearchResultsObserver(searchView);
	CTR->DlManager()->SetDownloadObserver(transfersView);
	CTR->SetConnectionObserverL(connectionsView);		
}

CSymellaAppUi::~CSymellaAppUi()
{
    delete iDecoratedTabGroup;
}

void CSymellaAppUi::HideTabGroup()
{
	iNaviPane->Pop(iDecoratedTabGroup);
}


void CSymellaAppUi::ShowTabGroupL()
{
	iNaviPane->PushL(*iDecoratedTabGroup);
}


TBool CSymellaAppUi::GetIapIdL(TInt32& aAccesPointId, TDes& aAccesPointName)
{
	LOG->WriteLineL(_L("[AppUi] Asking for access point"));
	TApaTask task(iEikonEnv->WsSession());
	task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
	task.BringToForeground();
	
	TInt32 iap = 0;
	TBuf<128> apName;
	
	CAccessPointSettingItem* iapSettingItem = 
		new (ELeave) CAccessPointSettingItem(1, iap, apName);
		
	CleanupStack::PushL(iapSettingItem);
	
	TBool changed = iapSettingItem->AskIapIdL(iap, aAccesPointName);
	
	CleanupStack::PopAndDestroy(); // iapSettingItem
	
	aAccesPointId = iap;
	return changed;
}


void CSymellaAppUi::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
{
	switch (aResourceId)
	{
	//	case R_SYMELLA_SEARCHVIEW_MENU:
	//	case R_SYMELLA_TRANSFERSVIEW_MENU:
		case R_SYMELLA_APP_MENU:
		{
			if (CTR->State() == CSAControl::EAutoConnect)
			{
				aMenuPane->SetItemDimmed(ESymellaCmdConnect, ETrue);
				aMenuPane->SetItemDimmed(ESymellaCmdDisconnect, EFalse);

				aMenuPane->SetItemDimmed(ESymellaCmdConnectInfo, EFalse);
			}
			else
			{
				aMenuPane->SetItemDimmed(ESymellaCmdDisconnect, ETrue);
				aMenuPane->SetItemDimmed(ESymellaCmdConnect, EFalse);

				aMenuPane->SetItemDimmed(ESymellaCmdConnectInfo, ETrue);
			}
		}
		break;

		default:
		break;
	}
}

// ----------------------------------------------------
// CSymellaAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
// ?implementation_description
// ----------------------------------------------------
//
TKeyResponse CSymellaAppUi::HandleKeyEventL(
    const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
    {
    if ( iTabGroup == NULL )
        {
        return EKeyWasNotConsumed;
        }

    TInt active = iTabGroup->ActiveTabIndex();
    TInt count = iTabGroup->TabCount();

    switch ( aKeyEvent.iCode )
        {
        case EKeyLeftArrow:
            if ( active > 0 )
                {
                active--;
                iTabGroup->SetActiveTabByIndex( active );
                ActivateLocalViewL(TUid::Uid(iTabGroup->TabIdFromIndex(active)));
                }
            break;
        case EKeyRightArrow:
            if( (active + 1) < count )
                {
                active++;
                iTabGroup->SetActiveTabByIndex( active );
                ActivateLocalViewL(TUid::Uid(iTabGroup->TabIdFromIndex(active)));
                }
            break;
        default:
            return EKeyWasNotConsumed;
            break;
        }

    return EKeyWasConsumed;
    }

// ----------------------------------------------------
// CSymellaAppUi::HandleCommandL(TInt aCommand)
// ?implementation_description
// ----------------------------------------------------
//
void CSymellaAppUi::HandleCommandL(TInt aCommand)
{
    switch ( aCommand )
    {
		case ESymellaCmdConnect:
		{
			CTR->ConnectL();
		}
		break;

		case ESymellaCmdSelectGenre:
		{
			TInt index = 0;
			CAknListQueryDialog* queryDialog = new(ELeave) CAknListQueryDialog(&index);
			if(queryDialog->ExecuteLD(R_SYMELLA_GENRE_SELECTION_LIST))
			{
				CTR->SetGenre(index);
			}
		}
		break;

		case ESymellaCmdResetHostCache:
		{
			// ask to quit
			CAknQueryDialog* query = CAknQueryDialog::NewL();
			CleanupStack::PushL(query);
			_LIT(KPrompt, "Are you sure you want to reset the hostcache?");
			query->SetPromptL(KPrompt);
			CleanupStack::Pop(); // query

			if (query->ExecuteLD(R_GENERAL_QUERY))
			{				
				CTR->HostCache().Reset();				
			}		

		}
		break;

		case ESymellaCmdDisconnect:
		{
			CTR->DisconnectL();
		}
		break;

		case ESymellaCmdConnectInfo:
		{
			HBufC* info = CTR->CreateConnectInfoL();
			CleanupStack::PushL(info);

			/*CAknNoteDialog* note = new (ELeave) CAknNoteDialog;
			CleanupStack::PushL(note);
			note->SetTextL(*info);
			CleanupStack::Pop();
			note->PrepareLC(R_CONNECTION_INFO_NOTE);
			note->RunLD();*/
			CAknMessageQueryDialog* dlg = new (ELeave)CAknMessageQueryDialog();
			dlg->PrepareLC( R_MESSAGE_QUERY );
			dlg->SetMessageTextL(info->Des());
			dlg->QueryHeading()->SetTextL(_L("Connect info"));
			dlg->RunLD();

			CleanupStack::PopAndDestroy(); //info
		}
		break;

		case ESymellaCmdAbout:
		{
			TBuf<30> time;
			TBuf<50> date;
			date.Copy(_L8(__DATE__));
			time.Copy(_L8(__TIME__));
			_LIT(KBuild, "Built on ");
			HBufC* info = HBufC::NewLC(TPtrC(KAbout).Length() + 64 /*+ TPtrC(KEngineVersion).Length() */+
				TPtrC(KBuild).Length() + date.Length() + 4);
			TPtr des = info->Des();
			des.Append(_L("Version "));
			des.Append(SYMELLA_VERSION_LIT);
			des.Append(KAbout);
			//des.Append(KEngineVersion);
			des.Append(KBuild);
			des.Append(date);

			CAknMessageQueryDialog* dlg = new (ELeave)CAknMessageQueryDialog();
			CleanupStack::PushL(dlg);
			dlg->PrepareLC( R_MESSAGE_QUERY );
			dlg->SetMessageTextL(des);
			dlg->QueryHeading()->SetTextL(_L("Symella S60"));
			CleanupStack::Pop(); //dlg
			dlg->RunLD();

			CleanupStack::PopAndDestroy(); //info
		}
		break;

		case ESymellaCmdAddNode:
            {
				// Create dialog to allow user to view/edit connection details
				TBuf<100> hostName;
				//hostName.Copy(_L(""));
			          hostName.Copy(_L("192.168.0.100"));
				TInt port = 6346;

				CAknMultiLineDataQueryDialog* dialog =
					CAknMultiLineDataQueryDialog::NewL(hostName, port);

				// Display and execute dialog, and act according to return value
				if (dialog->ExecuteLD(R_ADD_NODE_DIALOG))
				{
					TInetAddr addr;

					if (addr.Input(hostName) == KErrNone)
					{
						addr.SetPort(port);
						CTR->HostCache().AddUltrapeerL(addr);
					}                              
				}
            }
            break;
           
    	case EAknCmdExit:
        case EEikCmdExit:
        {
        	PREFERENCES->SaveWebCachesL();
    		LOG->WriteLineL(_L("Saving hostcache..."));
			CTR->HostCache().SaveHostCacheL();
			LOG->WriteLineL(_L("Symella terminating"));
			Exit();	
        }
    	break;
    	
    	case EAknSoftkeyBack:
		case EAknSoftkeyExit:
		{			
        	// ask to quit
			CAknQueryDialog* query = CAknQueryDialog::NewL();
			CleanupStack::PushL(query);
			_LIT(KPrompt, "Quit Symella?");
			query->SetPromptL(KPrompt);
			CleanupStack::Pop(); // query

			if (query->ExecuteLD(R_GENERAL_QUERY))
			{
			//	((CSymellaSearchView*)View(TUid::Uid(1)))->CloseFindBoxL();
				PREFERENCES->SaveWebCachesL();
				LOG->WriteLineL(_L("Saving hostcache..."));
				CTR->HostCache().SaveHostCacheL();
				LOG->WriteLineL(_L("Symella terminating"));
				Exit();								
			}		
        }			
        break;

		case ESymellaCmdSearch:
		{
			TBuf<256> query;
			query.Copy(CTR->SearchString());
			CAknTextQueryDialog* dlg = 
				new (ELeave) CAknTextQueryDialog(query, CAknQueryDialog::ENoTone);

			dlg->SetPredictiveTextInputPermitted(ETrue);
			if (! dlg->ExecuteLD(R_SEARCH_QUERY)) 
				break;
			
			HBufC8* query8 = HBufC8::NewLC(query.Length());
			TPtr8 query8Ptr(query8->Des());
			query8Ptr.Copy(query);
			CTR->SearchL(*query8);
			CleanupStack::PopAndDestroy(); // query8
			
			iTabGroup->SetActiveTabByIndex(1);
            ActivateLocalViewL(TUid::Uid(iTabGroup->TabIdFromIndex(1)));

			LOG->WriteL(_L("Searching for: "));
			LOG->WriteLineL(query);
		}
		break;

		case ESymellaCmdSettings:
		{
			ActivateLocalViewL(KSettingsViewId);
		}
		break;

        default:
            break;      
    }
}

void CSymellaAppUi::ReplaceSearchTextL(TInt aCounter)
{
	//if (iTabGroup->ActiveTabIndex() == 1)
	//{
		if (aCounter==-1)
			iTabGroup->ReplaceTabTextL(2, _L("Search"));
		else
		{
			TBuf<30> buf;
			buf.Format(_L("Search %u"),aCounter);
			iTabGroup->ReplaceTabTextL(2,buf);
		}
	//}
}

void CSymellaAppUi::HandleResourceChangeL(TInt aType) 
{ 
    CAknAppUi::HandleResourceChangeL( aType ); 
    
    if ( iTabGroup == NULL )
    	return;
    
	#ifdef EKA2
   	if ( aType == KAknsMessageSkinChange || aType == KEikDynamicLayoutVariantSwitch ) 
    { 
		TInt activaViewId = iTabGroup->TabIdFromIndex(iTabGroup->ActiveTabIndex());
    	View(TUid::Uid(activaViewId))->HandleViewRectChange();
    	View(TUid::Uid(activaViewId))->Redraw();
    } 
	#endif 
} 

// End of File  
