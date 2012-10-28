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
*  Name     : CSymellaTransfersContainer from SymellaContainer2.cpp
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include "SymellaTransfersContainer.h"
#include "SAControl.h"
#include "SADownloadManager.h"
#include <aknlists.h>
#include <barsread.h>
#include <aknmessagequerydialog.h>
#include <Symella.rsg>
#include "SymellaTransfersView.h"
#include "symella.hrh"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaTransfersContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymellaTransfersContainer::ConstructL(const TRect& aRect, CSymellaTransfersView* aView)
{
	iView = aView;
    CreateWindowL();

    iListBox = new (ELeave) CAknDoubleStyleListBox();
	iListBox->SetMopParent(this);
	iListBox->ConstructL(this,EAknListBoxSelectionList);
	iListBox->SetContainerWindowL(*this);	
	
	iListBox->View()->SetListEmptyTextL(_L("(no downloads)"));

	//create scrollbar
	iListBox->CreateScrollBarFrameL();
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
		CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);

    SetRect(aRect);
    ActivateL();
}

//handle keys
TKeyResponse CSymellaTransfersContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,	TEventCode aType)
{
if (aType == EEventKey) 
	{
		switch (aKeyEvent.iCode) 
		{
			case EKeyUpArrow:
			case EKeyDownArrow:
				return iListBox->OfferKeyEventL(aKeyEvent, aType);

			case EKeyDevice3: //EKeyOk
				{
					TInt index = iListBox->CurrentItemIndex();
					if ( index >= 0)
					{
						if (CTR->DlManager()->GetDownloadState(index) == CSADownload::EComplete)
						{
							iView->HandleCommandL(ESymellaCmdOpenWithApp);
						}
						else
						{
							HBufC* info = CTR->DlManager()->CreateDownloadStatusInfoL(index);
							CleanupStack::PushL(info);
							
							CAknMessageQueryDialog* dlg = new (ELeave)CAknMessageQueryDialog();
							CleanupStack::PushL(dlg);
							dlg->PrepareLC( R_MESSAGE_QUERY );
							dlg->SetMessageTextL(info->Des());
							dlg->QueryHeading()->SetTextL(_L("Download info"));
							CleanupStack::Pop();
							dlg->RunLD();

							CleanupStack::PopAndDestroy();
						}
					}
				}
				break;

			case EKeyBackspace:
			case EKeyDelete:
				{
					iView->HandleCommandL(ESymellaCmdRemoveDownload);
				}
				break;

			default:;
		}
	}

	return EKeyWasNotConsumed;
}

// Destructor
CSymellaTransfersContainer::~CSymellaTransfersContainer()
{
    delete iListBox;
}

// ---------------------------------------------------------
// CSymellaTransfersContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSymellaTransfersContainer::SizeChanged()
{    
    iListBox->SetRect(Rect());
}

// ---------------------------------------------------------
// CSymellaTransfersContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSymellaTransfersContainer::CountComponentControls() const
{
    return 1;
}

// ---------------------------------------------------------
// CSymellaTransfersContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSymellaTransfersContainer::ComponentControl(TInt aIndex) const
{
    switch ( aIndex )
    {
        case 0:
            return iListBox;;
        default:
            return NULL;
    }
}


// End of File  
