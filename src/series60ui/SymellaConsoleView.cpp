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
*  Name     : CSymellaConsoleView from SymellaConsoleView.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <eikedwin.h>
#include <Symella.rsg>
#include "SymellaConsoleView.h"
#include "SymellaConsoleContainer.h"
#include "SymellaAppUi.h"
#include <txtetext.h>
#include <eikenv.h>
#include <aknappui.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaConsoleView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymellaConsoleView::ConstructL()
{
    BaseConstructL( R_SYMELLA_CONSOLEVIEW );
	
    iContainer = new (ELeave) CSymellaConsoleContainer;
    iContainer->SetMopParent(this);
    iContainer->ConstructL( ClientRect(), iTextBuffer );
	iContainer->MakeVisible(EFalse);

//	iTextBuffer = CPlainText::NewL();
}

// ---------------------------------------------------------
// CSymellaConsoleView::~CSymellaConsoleView()
// ?implementation_description
// ---------------------------------------------------------
//
CSymellaConsoleView::~CSymellaConsoleView()
{
    if ( iContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iContainer );
    }

    delete iContainer;

	//delete iTextBuffer;
}

// ---------------------------------------------------------
// TUid CSymellaConsoleView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CSymellaConsoleView::Id() const
{
    return KConsoleViewId;
}

// ---------------------------------------------------------
// CSymellaConsoleView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaConsoleView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
    {
        case EAknSoftkeyOk:
        {
            iEikonEnv->InfoMsg( _L("view1 ok") );
            break;
        }
        case EAknSoftkeyBack:
        {
            AppUi()->HandleCommandL(EEikCmdExit);
            break;
        }
        default:
        {
            AppUi()->HandleCommandL( aCommand );
            break;
        }
     }
}

void CSymellaConsoleView::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
{
	static_cast<CAknAppUi*>(CEikonEnv::Static()->EikAppUi())->
		DynInitMenuPaneL(aResourceId, aMenuPane);
}

// ---------------------------------------------------------
// CSymellaConsoleView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSymellaConsoleView::HandleViewRectChange()
{
    if ( iContainer )
    {
        iContainer->SetRect( ClientRect() );
    }
}

// ---------------------------------------------------------
// CSymellaConsoleView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaConsoleView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
	iContainer->MakeVisible(ETrue);
	AppUi()->AddToStackL( *this, iContainer );
}

// ---------------------------------------------------------
// CSymellaConsoleView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaConsoleView::DoDeactivate()
{
    if ( iContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iContainer );
		iContainer->MakeVisible(EFalse);
    }
    
    //delete iContainer;
    //iContainer = NULL;
}


void CSymellaConsoleView::HandleLogWriteL(const TDesC& aText)
{
	/*if (aText.Length() < KMaxConsoleBufferSize)
	{
		// Checks the new text length
		TInt diff = iTextBuffer->DocumentLength() + 
			aText.Length() - KMaxConsoleBufferSize;

		// If it's longer than the maximum then deletes from
		// the beginning
		if ( diff > 0)
		{
			iTextBuffer->DeleteL(0, diff);
			if (iContainer)
				iContainer->Edwin()->Text()->DeleteL(0, diff);
		}

		iTextBuffer->InsertL(iTextBuffer->DocumentLength(), aText);*/

		if (iContainer)
		{
			CEikEdwin* edwin = iContainer->Edwin();

			TInt diff = edwin->Text()->DocumentLength() + 
				aText.Length() - KMaxConsoleBufferSize;

			if (diff > edwin->Text()->DocumentLength()) 
				diff = edwin->Text()->DocumentLength();

			if ( diff > 0)			
				edwin->Text()->DeleteL(0, diff);

			
			edwin->Text()->InsertL(edwin->Text()->DocumentLength(), aText);
			edwin->HandleTextChangedL();
			edwin->SetCursorPosL(edwin->Text()->DocumentLength(), EFalse);		
		}
/*	}
	else
	{
		TPtrC ptr = aText.Right(KMaxConsoleBufferSize);

		iTextBuffer->DeleteL(0, iTextBuffer->DocumentLength());
		iTextBuffer->InsertL(0, ptr);
		if (iContainer)
		{
			CEikEdwin* edwin = iContainer->Edwin();
			edwin->Text()->DeleteL(0, edwin->Text()->DocumentLength());
			edwin->Text()->InsertL(0, ptr);
			edwin->HandleTextChangedL();
			edwin->SetCursorPosL(edwin->Text()->DocumentLength(), EFalse);		
		}		
	}*/
}

// End of File

