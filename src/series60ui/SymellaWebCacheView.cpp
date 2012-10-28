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

// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <caknfileselectiondialog.h>
#include <caknmemoryselectiondialog.h> 
#include <pathinfo.h> 
#include <aknnotewrappers.h>
#include <aknmessagequerydialog.h>
#include <DocumentHandler.h>
#include <apmstd.h>
#include "SymellaWebCacheView.h"
#include "SymellaWebCacheContainer.h" 
#include <Symella.rsg>
#include "Symella.hrh"


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaWebCacheView::ConstructL(CSTTorrentManager* aTorrentMgr, 
//   CSymTorrentAppUi* aAppUi)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymellaWebCacheView::ConstructL(CSymellaAppUi* aAppUi)
{
	iAppUi = aAppUi;
    BaseConstructL( R_SYMELLA_WEBCACHEVIEW );
    
    PREFERENCES->SetWebCacheListObserver(this);
}

// ---------------------------------------------------------
// CSymellaWebCacheView::~CSymellaWebCacheView()
// destructor
// ---------------------------------------------------------
//
CSymellaWebCacheView::~CSymellaWebCacheView()
{
    if ( iContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iContainer );
    }
    
    delete iContainer;
    iContainer = NULL;
}

// ---------------------------------------------------------
// TUid CSymellaWebCacheView::Id()
//
// ---------------------------------------------------------
//
TUid CSymellaWebCacheView::Id() const
{
    return KWebCacheViewId;
}

// ---------------------------------------------------------
// CSymellaWebCacheView::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSymellaWebCacheView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
        {
        case ESymellaCmdAddWebcache:
            {
            TBuf<256> query;
			query.Copy(_L("http://"));
			CAknTextQueryDialog* dlg = 
				new (ELeave) CAknTextQueryDialog(query, CAknQueryDialog::ENoTone);

			dlg->SetPredictiveTextInputPermitted(ETrue);
			if (! dlg->ExecuteLD(R_ADD_WEBCACHE_QUERY)) 
				break;
			
			TBuf8<256> query8;
			query8.Copy(query);
			PREFERENCES->AddWebCacheL(query8);
			
            break;
            }
        case ESymellaCmdRemoveWebcache:
            {
            if (iContainer->ListBox()->CurrentItemIndex() >= 0)
            {
	            CAknQueryDialog* query = CAknQueryDialog::NewL();
				CleanupStack::PushL(query);
				_LIT(KPrompt, "Are you sure you want to remove webcache?");
				query->SetPromptL(KPrompt);
				CleanupStack::Pop(); // query

				if (query->ExecuteLD(R_GENERAL_QUERY))
				{	
					PREFERENCES->RemoveWebCacheL(iContainer->ListBox()->CurrentItemIndex());			
	            }
            }
            	
            break;
            }
        case ESymellaCmdWebcacheDetails:
            {
            if (iContainer->ListBox()->CurrentItemIndex() >= 0)
            {
            	CSAPreferences* prefs = PREFERENCES;
	            CSAWebCacheListEntry* cache = prefs->WebCache(iContainer->ListBox()->CurrentItemIndex());
	            HBufC* info = HBufC::NewLC(cache->Address().Length());
	            TPtr infoPtr(info->Des());
	            
	            infoPtr.Copy(cache->Address());
				
				CAknMessageQueryDialog* dlg = new (ELeave)CAknMessageQueryDialog();
				CleanupStack::PushL(dlg);
				dlg->PrepareLC( R_MESSAGE_QUERY );
				dlg->SetMessageTextL(*info);
				dlg->QueryHeading()->SetTextL(_L("Webcache details"));
				CleanupStack::Pop();
				dlg->RunLD();

				CleanupStack::PopAndDestroy(); // info
            }
         
            		
            break;
            }
        case EAknSoftkeyBack:
            {
			AppUi()->ActivateLocalViewL(KSettingsViewId);
            break;
            }
        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }
}

void CSymellaWebCacheView::WebCacheListChangedL()
{
	if (iContainer)
	{
		iContainer->RefreshListL();
	}
}

// ---------------------------------------------------------
// CSymellaWebCacheView::HandleViewRectChange()
// ---------------------------------------------------------
//
void CSymellaWebCacheView::HandleViewRectChange()
{
	if ( iContainer )
	{
		iContainer->SetRect( ClientRect() );
	}
}

// ---------------------------------------------------------
// CSymellaWebCacheView::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSymellaWebCacheView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{	
    if (!iContainer)
        {
        static_cast<CSymellaAppUi*>(AppUi())->HideTabGroup();
        
        iContainer = new (ELeave) CSymellaWebCacheContainer;
        iContainer->SetMopParent(this);
        iContainer->ConstructL( ClientRect() , iAppUi);
        AppUi()->AddToStackL( *this, iContainer );		
        }
}

// ---------------------------------------------------------
// CSymellaWebCacheView::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSymellaWebCacheView::DoDeactivate()
{
    if ( iContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        if (!AppUi()->View(KSettingsViewId)->IsForeground())
        	static_cast<CSymellaAppUi*>(AppUi())->ShowTabGroupL();
    }
     
    delete iContainer;
    iContainer = NULL;	
}

void CSymellaWebCacheView::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
{
	switch (aResourceId)
	{
		case R_SYMELLA_WEBCACHEVIEW_MENU:	
		{
			
		/*	if (index < 0)			
				aMenuPane->SetItemDimmed(ESymTorrentCmdFileDetails, ETrue);		
			else
				aMenuPane->SetItemDimmed(ESymTorrentCmdFileDetails, EFalse);
			
			if ((index < 0) || (!file->IsDownloaded()))
				aMenuPane->SetItemDimmed(ESymTorrentCmdOpenFile, ETrue);
			else
				aMenuPane->SetItemDimmed(ESymTorrentCmdOpenFile, EFalse);*/
		}
		break;

		default:
		break;
	}
	
	AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
}
