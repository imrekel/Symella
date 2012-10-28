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
#include "SymellaWebCacheContainer.h"

#ifdef EKA2
#include <AknsUtils.h>
#endif

//#include <AknsDrawUtils.h>// skin
//#include <AknsBasicBackgroundControlContext.h> //skin 
//#include <akniconarray.h> 
#include <aknnotewrappers.h>
#include <eikclbd.h>
#include <aknlists.h>
#include <eikenv.h>
#include "SymellaWebCacheView.h"
#include "Symella.hrh"
#include "SAPreferences.h"



// ================= MEMBER FUNCTIONS =======================

void CSymellaWebCacheContainer::ConstructL(const TRect& aRect, 
	CSymellaAppUi* aAppUi)
{
	iAppUi = aAppUi;	
	iPreferences = PREFERENCES;
	
    CreateWindowL();
       
    iListBox = new (ELeave) CAknSingleNumberStyleListBox();
    iListBox->SetMopParent(this);
	iListBox->ConstructL(this,EAknListBoxSelectionList);
	iListBox->SetContainerWindowL(*this);	

	// create scrollbar
	iListBox->CreateScrollBarFrameL();
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
		CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
	
    SetRect(aRect);
   // iBackGround = CAknsBasicBackgroundControlContext::NewL( KAknsIIDQsnBgAreaMain, Rect(), EFalse );
    ActivateL();
    
    RefreshListL();

//	iListBox->SetCurrentItemIndexAndDraw(itemArray->Count() - 1);
}

void CSymellaWebCacheContainer::RefreshListL()
{
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iListBox->Model()->ItemTextArray());
	
	TBool repositionHighlight = EFalse;	
	if (iListBox->CurrentItemIndex() > (iPreferences->WebCacheCount() - 1))
		repositionHighlight = ETrue;
	
	itemArray->Reset();
	
	for (TInt i=0; i<iPreferences->WebCacheCount(); i++)
	{	
		HBufC* item = HBufC::NewLC(iPreferences->WebCache(i)->Address().Length() + 10);
		TPtr itemPtr(item->Des());
			
		TPtrC8 address = iPreferences->WebCache(i)->Address();
		if (address.Left(7) == _L8("http://"))
			itemPtr.Copy(address.Mid(7));
		else
			itemPtr.Copy(address);
		itemPtr.Insert(0, _L("\t"));
		
		TBuf<16> numBuf;
		numBuf.Num(i + 1);
		itemPtr.Insert(0, numBuf);
		
		itemArray->AppendL(*item);
							
		CleanupStack::PopAndDestroy(); // item				
	}

	iListBox->HandleItemAdditionL();
	
	if (repositionHighlight && (iPreferences->WebCacheCount() > 0))
		iListBox->SetCurrentItemIndex(iPreferences->WebCacheCount()-1);
	
	iListBox->DrawDeferred();
}

// Destructor
CSymellaWebCacheContainer::~CSymellaWebCacheContainer()
{
    delete iListBox;
}

// ---------------------------------------------------------
// CSymellaWebCacheContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSymellaWebCacheContainer::SizeChanged()
{
    iListBox->SetRect(Rect());
}

// ---------------------------------------------------------
// CSymellaWebCacheContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSymellaWebCacheContainer::CountComponentControls() const
{
	return 1;
}

// ---------------------------------------------------------
// CSymellaWebCacheContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSymellaWebCacheContainer::ComponentControl(TInt aIndex) const
{
	switch ( aIndex )
		{
		case 0:
			return iListBox;
		default:
			return NULL;
		}
}

// ---------------------------------------------------------
// CSymellaWebCacheContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CSymellaWebCacheContainer::Draw(const TRect& /*aRect*/) const
{
/*	CWindowGc& gc = SystemGc();
	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
	AknsDrawUtils::Background( skin, cc, this, gc, aRect );*/
}

// ---------------------------------------------------------
// CSymellaWebCacheContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CSymellaWebCacheContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
{
    // TODO: Add your control event handler code here
}

// ---------------------------------------------------------
// CSymellaWebCacheContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,
//	 TEventCode aType)
// ---------------------------------------------------------
//
TKeyResponse CSymellaWebCacheContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType)
{
	 switch ( aKeyEvent.iCode )
	 {
	 	case EKeyDownArrow:
	 	case EKeyRightArrow:
	 	case EKeyLeftArrow:	
		case EKeyUpArrow:
		{
			iListBox->OfferKeyEventL(aKeyEvent,aType);
			return EKeyWasConsumed;
			break;
		}
	
		case EKeyOK: 
		{
			static_cast<CSymellaWebCacheView*>(iAppUi->View(KWebCacheViewId))
				->HandleCommandL(ESymellaCmdWebcacheDetails);
				
			return EKeyWasConsumed;
			break;
		}
		
		case EKeyBackspace:
		case EKeyDelete:
		{
			static_cast<CSymellaWebCacheView*>(iAppUi->View(KWebCacheViewId))
				->HandleCommandL(ESymellaCmdRemoveWebcache);
		}
		break;
		
	    default:
		  return EKeyWasNotConsumed;		
	 }
	 return EKeyWasNotConsumed;
}

// ---------------------------------------------------------
// CSymellaWebCacheContainer::MopSupplyObject()
// Pass skin information if needed.
// ---------------------------------------------------------
//
TTypeUid::Ptr CSymellaWebCacheContainer::MopSupplyObject(TTypeUid aId)
{
   /* if(aId.iUid == MAknsControlContext::ETypeId && iBackGround)
        {
        return MAknsControlContext::SupplyMopObject( aId, iBackGround);
        }*/

    return CCoeControl::MopSupplyObject( aId );
}


// ---------------------------------------------------------
// CSymellaWebCacheContainer::TorrentListBox()
// Returns the listBox;
// ---------------------------------------------------------
//
CAknSingleNumberStyleListBox* CSymellaWebCacheContainer::ListBox()
{
	return iListBox;
}


// End of File  
