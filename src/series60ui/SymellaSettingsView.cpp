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
*  Name     : CSymellaSettingsView from SymellaView2.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include  <aknviewappui.h>
#include  <avkon.hrh>
#include  <Symella.rsg>
#include  "SymellaSettingsView.h"
#include  "SymellaSettingsContainer.h" 
#include  "SymellaAppUi.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaSettingsView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymellaSettingsView::ConstructL()
    {
	BaseConstructL( R_SYMELLA_SETTINGSVIEW );
    }

// ---------------------------------------------------------
// CSymellaSettingsView::~CSymellaSettingsView()
// ?implementation_description
// ---------------------------------------------------------
//
CSymellaSettingsView::~CSymellaSettingsView()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    }

// ---------------------------------------------------------
// TUid CSymellaSettingsView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CSymellaSettingsView::Id() const
    {
    return KSettingsViewId;
    }

// ---------------------------------------------------------
// CSymellaSettingsView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaSettingsView::HandleCommandL(TInt aCommand)
    {   
    switch ( aCommand )
        {
        case EAknSoftkeySelect:
            {
            iContainer->EditItemL(
				iContainer->ListBox()->CurrentItemIndex(), EFalse);
            break;
            }
        case EAknSoftkeyCancel:
            {
			AppUi()->ActivateLocalViewL(iPrevViewId.iViewUid);
            break;
            }
        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }
    }

// ---------------------------------------------------------
// CSymellaSettingsView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSymellaSettingsView::HandleViewRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CSymellaSettingsView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaSettingsView::DoActivateL(
   const TVwsViewId& aPrevViewId,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
    {
	

    if (!iContainer)
        {
        if (aPrevViewId != AppUi()->View(KWebCacheViewId)->ViewId())
			iPrevViewId = aPrevViewId;

		static_cast<CSymellaAppUi*>(AppUi())->HideTabGroup();

        iContainer = new (ELeave) CSymellaSettingsContainer;
        iContainer->SetMopParent(this);
		iContainer->GetPreferencesL();
		iContainer->ConstructL( ClientRect() );
        AppUi()->AddToStackL( *this, iContainer );
        }
   }

// ---------------------------------------------------------
// CSymellaSettingsView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaSettingsView::DoDeactivate()
    {
    if ( iContainer )
        {      
        AppUi()->RemoveFromViewStack( *this, iContainer );
        
        if (!AppUi()->View(KWebCacheViewId)->IsForeground())
			static_cast<CSymellaAppUi*>(AppUi())->ShowTabGroupL();
        }
    
    delete iContainer;
    iContainer = NULL;
    }

// End of File

