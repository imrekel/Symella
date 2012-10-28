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
*  Name     : CSymellaConnectionsContainer from SymellaContainer2.cpp
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include "SymellaConnectionsContainer.h"

#include <aknlists.h>
#include <barsread.h>
#include <Symella.rsg>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaConnectionsContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymellaConnectionsContainer::ConstructL(const TRect& aRect)
{
    CreateWindowL();

    iListBox = new (ELeave) CAknSingleHeadingStyleListBox();
	iListBox->SetMopParent(this);
	iListBox->ConstructL(this,EAknListBoxSelectionList);
	iListBox->SetContainerWindowL(*this);

	//TResourceReader reader; //resource reader
	//iCoeEnv->CreateResourceReaderLC(reader, R_TRANSFERS_LISTBOX); //read in the transfers listbox
	//iListBox->ConstructFromResourceL(reader);
	//CleanupStack::PopAndDestroy(); //reader

	//create scrollbar
	iListBox->CreateScrollBarFrameL();
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
		CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);

    SetRect(aRect);
    ActivateL();
}

//handle keys
TKeyResponse CSymellaConnectionsContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,	TEventCode aType)
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
				
			}
			break;
		default:;
		}
	}

	return EKeyWasNotConsumed;
}

// Destructor
CSymellaConnectionsContainer::~CSymellaConnectionsContainer()
{
    delete iListBox;
}

// ---------------------------------------------------------
// CSymellaConnectionsContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSymellaConnectionsContainer::SizeChanged()
{    
    iListBox->SetRect(Rect());
}

// ---------------------------------------------------------
// CSymellaConnectionsContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSymellaConnectionsContainer::CountComponentControls() const
{
    return 1;
}

// ---------------------------------------------------------
// CSymellaConnectionsContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSymellaConnectionsContainer::ComponentControl(TInt aIndex) const
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

