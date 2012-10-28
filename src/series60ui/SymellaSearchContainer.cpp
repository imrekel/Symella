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
*  Name     : CSymellaSearchContainer from SymellaContainer2.cpp
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include "SymellaSearchContainer.h"
#include "SAControl.h"
#include "SADownloadManager.h"
#include <Symella.rsg>
#include <aknnotedialog.h>
#include <aknmessagequerydialog.h>
#include <aknlists.h>

// CONSTS
const TInt KListFindBoxTextLength = 64;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaSearchContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymellaSearchContainer::ConstructL(const TRect& aRect)
{
    CreateWindowL();

    iListBox = new (ELeave) CAknSingleNumberStyleListBox();
	iListBox->SetMopParent(this);
	iListBox->ConstructL(this,EAknListBoxSelectionList);
	iListBox->SetContainerWindowL(*this);
	
	TPtrC8 searcString8 = CTR->SearchString();
	
	if (CTR->SearchString() != KNullDesC8)
	{
		HBufC* searchString = HBufC::NewLC(searcString8.Length() + 30);
		TPtr searchStringPtr (searchString->Des());
		searchStringPtr.Copy(searcString8);
		searchStringPtr.Insert(0, _L("(searching for \""));
		searchStringPtr.Append(_L("\")"));
		iListBox->View()->SetListEmptyTextL(searchStringPtr);
		CleanupStack::PopAndDestroy(); // searchString		
	}
	else
		iListBox->View()->SetListEmptyTextL(_L("(no active search)"));

	iListBox->CreateScrollBarFrameL();
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
		CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
		

	CreateFindBoxL();

    SetRect(aRect);
    ActivateL();
}

void CSymellaSearchContainer::CreateFindBoxL()
{
	DeleteFindBox();
	
		// Sets style of findbox.
    CAknSearchField::TSearchFieldStyle style( 
        CAknSearchField::ESearch );
    style = CAknSearchField::EPopup;   
    /*if ( flagsOfPopup )
    {
    	style = CAknSearchField::EPopup;
    }*/

    // Creates FindBox.
    iFindBox = CreateFindBoxL( iListBox, iListBox->Model(), style );

    //if ( iFindBox && style == CAknSearchField::EPopup )
    //{
	    // Popup findbox is not shown when listbox display initial.
	//    iFindBox->MakeVisible( EFalse );
    //}
//	iFindBox->SetFocus(ETrue);
	iFindBox->MakeVisible(EFalse);
	
//	iFindBox->ActivateL();
	
}

void CSymellaSearchContainer::DeleteFindBox()
{
	delete iFindBox;
	iFindBox = 0;
}

// Destructor
CSymellaSearchContainer::~CSymellaSearchContainer()
{
	iFindBox->SetFocus(EFalse);
	delete iFindBox;
    delete iListBox;
}

// ---------------------------------------------------------
// CSymellaSearchContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSymellaSearchContainer::SizeChanged()
{    
    if ( iFindBox )
	{
		// Calls HandleFindSizeChanged after creates findbox.
		AknFind::HandlePopupFindSizeChanged(this, iListBox, iFindBox);
	}
	else
	{
		iListBox->SetRect( Rect() ); // Sets rectangle of lstbox.
	}
}

// ---------------------------------------------------------
// CSymellaSearchContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSymellaSearchContainer::CountComponentControls() const
{
	if (iFindBox)
    	return 2;
	else
		return 1;
}


CAknSearchField* CSymellaSearchContainer::CreateFindBoxL( 
    CEikListBox* aListBox,
    CTextListBoxModel* aModel,
    CAknSearchField::TSearchFieldStyle aStyle )
{
	CAknSearchField* findbox = NULL;

	if ( aListBox && aModel )
	    {
	    // Gets pointer of CAknFilteredTextListBoxModel.
	    CAknFilteredTextListBoxModel* model =
	            static_cast<CAknFilteredTextListBoxModel*>( aModel );
	    // Creates FindBox.
	    findbox = CAknSearchField::NewL( *this,
	                                     aStyle,
	                                     NULL,
	                                     KListFindBoxTextLength );
	    CleanupStack::PushL( findbox );
	    // Creates CAknListBoxFilterItems class.
	    model->CreateFilterL( aListBox, findbox );
	    //Filter can get by model->Filter();
	    CleanupStack::Pop( findbox ); // findbox
	    }

	return findbox;
}


// ---------------------------------------------------------
// CSymellaSearchContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSymellaSearchContainer::ComponentControl(TInt aIndex) const
{
    switch ( aIndex )
    {
        case 0:
            return iListBox;
        case 1:
        	if (iFindBox)
            	return iFindBox;
        default:
            return NULL;
    }
}


TInt CSymellaSearchContainer::CurrentItemIndex()
{
	if ((iListBox->CurrentItemIndex() >= 0) 
		&& (iFindBox) 
		&& (iFindBox->TextLength() > 0))
	{
		CAknFilteredTextListBoxModel* model =
                static_cast<CAknFilteredTextListBoxModel*>(iListBox->Model());
                
        CAknListBoxFilterItems* filter = model->Filter(); 
        
        return filter->FilteredItemIndex(iListBox->CurrentItemIndex());
	}	        
    else
    	return iListBox->CurrentItemIndex();

}


TKeyResponse CSymellaSearchContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
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
				if (CurrentItemIndex() >= 0)
				{
					HBufC* info = CTR->DlManager()->CreateHitInfoL(CurrentItemIndex());
					CleanupStack::PushL(info);

					/*CAknNoteDialog* note = new (ELeave) CAknNoteDialog;
					CleanupStack::PushL(note);
					note->SetTextL(*info);
					CleanupStack::Pop();
					note->PrepareLC(R_HIT_INFO_NOTE);
					note->RunLD();*/
					CAknMessageQueryDialog* dlg = new (ELeave)CAknMessageQueryDialog();
					dlg->PrepareLC( R_MESSAGE_QUERY );
					dlg->SetMessageTextL(info->Des());
					dlg->QueryHeading()->SetTextL(_L("Hit info"));
					dlg->RunLD();

					CleanupStack::PopAndDestroy();
				}
			}
			break;
		case EKeyLeftArrow:
		case EKeyRightArrow:
		{
			// Empty FindBox Search Text		
			_LIT(KBlank, "");
			FindBox()->SetSearchTextL(KBlank);

			// Press Backspace on Empty FindBox to Close it
			TBool refresh = false;
			TKeyEvent key;
			key.iCode = 8;
			key.iScanCode = 1;
			key.iRepeats = 1;
			key.iModifiers = 0;
			AknFind::HandleFindOfferKeyEventL(key, EEventKey, this, ListBox(), FindBox(), ETrue, refresh);
		}
		break;
			
		default:
			{
				if (iFindBox)
				{
					

					TBool needRefresh( EFalse );
	            	TBool flagsOfPopup( EFalse );

					if ( AknFind::HandleFindOfferKeyEventL( aKeyEvent, aType, this,
		                                                    iListBox, iFindBox,
		                                                    ETrue,
		                                                    needRefresh ) ==
		                                                        EKeyWasConsumed )
		            {
		                if ( needRefresh )
		                {
		                	SizeChanged();
		                    DrawNow();
		                }

		                return EKeyWasConsumed;
		            }
				}
			}
			break;
		}
	}

	return EKeyWasNotConsumed;
}



// End of File  
