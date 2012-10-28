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
*  Name     : CSymellaConsoleContainer from SymellaConsoleContainer.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include "SymellaConsoleContainer.h"

#include <EikEdwin.h>
//#include 
#include <aknutils.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaConsoleContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymellaConsoleContainer::ConstructL(const TRect& aRect, CPlainText* /*aEdwinDocument*/)
{
    CreateWindowL();

    iEdwin = new (ELeave) CEikEdwin;
	iEdwin->SetContainerWindowL(*this);
	iEdwin->SetMopParent(this);
	iEdwin->ConstructL(EEikEdwinNoAutoSelection,0,0,0);
	iEdwin->SetReadOnly(ETrue);

	// Scrollbar
	iEdwin->CreateScrollBarFrameL();
	iEdwin->ScrollBarFrame()->SetScrollBarVisibilityL(
		CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);

	// Character Format
	{
		CCharFormatLayer* formatLayer = CEikonEnv::NewDefaultCharFormatLayerL();
		TCharFormat charFormat; 
		TCharFormatMask charFormatMask; 
		formatLayer->Sense(charFormat, charFormatMask);

		// Color
		charFormat.iFontPresentation.iTextColor = KRgbBlack;
		charFormatMask.SetAttrib(EAttColor);
		
		// Font
		charFormat.iFontSpec.iTypeface = 
			LatinPlain12()->FontSpecInTwips().iTypeface;
		charFormatMask.SetAttrib(EAttFontTypeface);

		// Height
	//	charFormat.iFontSpec.iHeight = 15;
	//	charFormatMask.SetAttrib(EAttFontHeight); 

		formatLayer->SetL(charFormat, charFormatMask);
		iEdwin->SetCharFormatLayer(formatLayer);
	}

	// Paragraph Format
	{
		CParaFormat* paraFormat = CParaFormat::NewLC();
		TParaFormatMask paraFormatMask;

		paraFormat->iLineSpacingControl = CParaFormat::ELineSpacingExactlyInPixels;
		paraFormatMask.SetAttrib(EAttLineSpacingControl);
	
		paraFormat->iLineSpacingInTwips = 16;
		paraFormatMask.SetAttrib(EAttLineSpacing);

		paraFormat->iSpaceBeforeInTwips = 0;
		paraFormatMask.SetAttrib(EAttSpaceBefore);	

		paraFormat->iSpaceAfterInTwips = 0;
		paraFormatMask.SetAttrib(EAttSpaceAfter);	

		CParaFormatLayer* paraFormatLayer = 
			CParaFormatLayer::NewL(paraFormat, paraFormatMask);

		CleanupStack::PopAndDestroy();

		iEdwin->SetParaFormatLayer(paraFormatLayer);
	}

	// initialize the edwins content with the supplied document
//	TPtrC ptr = aEdwinDocument->Read(0, aEdwinDocument->DocumentLength());
//	iEdwin->Text()->InsertL(0,ptr);

	iEdwin->SetFocus(ETrue);

    SetRect(aRect);
    ActivateL();

//	iEdwin->HandleTextChangedL();
//	iEdwin->SetCursorPosL(aEdwinDocument->DocumentLength(), EFalse);
}

// Destructor
CSymellaConsoleContainer::~CSymellaConsoleContainer()
{
    delete iEdwin;
}

// ---------------------------------------------------------
// CSymellaConsoleContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSymellaConsoleContainer::SizeChanged()
{
    iEdwin->SetRect(Rect());
}

// ---------------------------------------------------------
// CSymellaConsoleContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSymellaConsoleContainer::CountComponentControls() const
    {
    return 1; // return nbr of controls inside this container
    }

// ---------------------------------------------------------
// CSymellaConsoleContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSymellaConsoleContainer::ComponentControl(TInt aIndex) const
{
    switch ( aIndex )
        {
        case 0:
            return iEdwin;
        default:
            return NULL;
        }
}


TKeyResponse CSymellaConsoleContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
												TEventCode aType)
{
	if (aType == EEventKey) 
	{
		switch (aKeyEvent.iCode) 
		{
		case EKeyUpArrow:
		case EKeyDownArrow:
			return iEdwin->OfferKeyEventL(aKeyEvent, aType);
		default:;
		}
	}

	return EKeyWasNotConsumed;
}

// End of File  
