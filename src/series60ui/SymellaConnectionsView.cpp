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
*  Name     : CSymellaConnectionsView from SymellaView2.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include  <aknviewappui.h>
#include  <avkon.hrh>
#include  <Symella.rsg>
#include  "SymellaConnectionsView.h"
#include  "SymellaConnectionsContainer.h"
#include "SAControl.h"
#include "SymellaAppUi.h"
#include <aknlists.h>
#include <eikenv.h>
#include <aknappui.h>
#include <eikmenup.h>
#include "symella.hrh"

_LIT(KProgressIndicator, "|/-\\|/-\\");


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaConnectionsView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymellaConnectionsView::ConstructL()
    {
    BaseConstructL( R_SYMELLA_CONNECTIONSVIEW );

	iContainer = new (ELeave) CSymellaConnectionsContainer;
    iContainer->SetMopParent(this);
    iContainer->ConstructL( ClientRect() );
	iContainer->MakeVisible(EFalse);
	iContainer->SetRect(ClientRect());

	//add a hostcache item to the list
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());
	itemArray->AppendL(_L("\tHostCache"));
	iUltrapeerCount = 0;
	iLeafCount = 0;
	iWorkingNodesCount = 0;
	iUnidentifiedNodesCount = 0;
	iWebCacheIPCount = 0;
	SetHostCacheTextL();
    }

// ---------------------------------------------------------
// CSymellaConnectionsView::~CSymellaConnectionsView()
// ?implementation_description
// ---------------------------------------------------------
//
CSymellaConnectionsView::~CSymellaConnectionsView()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    }

// ---------------------------------------------------------
// TUid CSymellaConnectionsView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CSymellaConnectionsView::Id() const
    {
    return KConnectionsViewId;
    }

// ---------------------------------------------------------
// CSymellaConnectionsView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaConnectionsView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
    {
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

// ---------------------------------------------------------
// CSymellaConnectionsView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSymellaConnectionsView::HandleViewRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CSymellaConnectionsView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaConnectionsView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
	iContainer->SetRect( ClientRect() );
	iContainer->MakeVisible(ETrue);
    AppUi()->AddToStackL( *this, iContainer );
}

// ---------------------------------------------------------
// CSymellaConnectionsView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaConnectionsView::DoDeactivate()
{
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
		iContainer->MakeVisible(EFalse);
        }
    
    //delete iContainer;
    //iContainer = NULL;
}


void CSymellaConnectionsView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
{
/*	switch (aResourceId)
	{
		case R_SYMELLA_TRANSFERSVIEW_MENU:	
		{
			if (iContainer->ListBox()->CurrentItemIndex() < 0)
			{
				aMenuPane->SetItemDimmed(ESymellaCmdRemoveDownload, ETrue);
			}
			else
				aMenuPane->SetItemDimmed(ESymellaCmdRemoveDownload, EFalse);
		}
		break;

		default:
		break;
	}*/
	AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);	
}

void CSymellaConnectionsView::AddAddressL(const TInetAddr& aAddress, const TBool aIsSemPeer)
{
	//add the address to the end of the list, before hostcache
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	HBufC* text = HBufC::NewLC(50);
	TPtr des = text->Des();
	TBuf<30> ips;
	aAddress.Output(ips);
	des.Format(_L("wait\t%S"),&ips,aAddress.Port());
	if(aIsSemPeer)
		des.Append(_L("s"));
	TInt pos = itemArray->Count()-1;
	if (pos<0) pos = 0;

	itemArray->InsertL(pos, *text);
	iContainer->ListBox()->HandleItemAdditionL();

	CleanupStack::PopAndDestroy(); //text
}
void CSymellaConnectionsView::SetUltrapeerL(const TInetAddr& /*aAddress*/)
{
/*	//find address
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());
	TBuf<40> ips;
	TInt pos=-1;
	aAddress.Output(ips);
	for (TInt i=0; i<itemArray->Count(); i++)
	{
		if ((*itemArray)[i].Find(ips) != KErrNotFound)
			pos=i;
	}

	if (pos>-1)
	{
		//create new text
		HBufC* text = HBufC::NewLC(50);
		TPtr des = text->Des();
		des.Copy((*itemArray)[pos]);
		des.Append(_L("(U)"));

		itemArray->Delete(pos);

		TInt currentItemIndex = iContainer->ListBox()->CurrentItemIndex();
		TBool currentItemDeleted;
		if (currentItemIndex == pos)
			currentItemDeleted = ETrue;
		else
			currentItemDeleted = EFalse;

		//AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
		//	iContainer->ListBox(), currentItemIndex, currentItemDeleted);

		itemArray->InsertL(pos,*text);
		iContainer->ListBox()->HandleItemAdditionL();

		CleanupStack::PopAndDestroy(); //text;
	}*/

}
void CSymellaConnectionsView::SetStateL(const TInetAddr& aAddress, TSAConnectionState aState)
{
	//find address
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());
	TBuf<40> ips;
	TInt pos=-1;
	aAddress.Output(ips);
	for (TInt i=0; i<itemArray->Count(); i++)
	{
		if ((*itemArray)[i].Find(ips) != KErrNotFound)
			pos=i;
	}

	if (pos>-1)
	{
		//create new text
		HBufC* text = HBufC::NewLC(50);
		TPtr des = text->Des();
		des.Copy((*itemArray)[pos]);
		TInt open = des.Find(_L("\t"));
		
		//state
		if (aState == MSAConnectionObserver::EIdle) {des.Replace(0,open,_L("Idle"));}
		else if (aState == MSAConnectionObserver::EConnecting) {des.Replace(0,open,_L("Conn"));}
		else if (aState == MSAConnectionObserver::EHandshaking) {des.Replace(0,open,_L("Handsh"));}
		else if (aState == MSAConnectionObserver::EConnected) {des.Replace(0,open,_L("OK"));}
		else if (aState == MSAConnectionObserver::EClose) {des.Replace(0,open,_L("Close"));}
		else if (aState == MSAConnectionObserver::EError) {des.Replace(0,open,_L("Error"));}
		else if (aState == MSAConnectionObserver::EDisconnected) {des.Replace(0,open,_L("Disc"));}

		itemArray->Delete(pos);

		TInt currentItemIndex = iContainer->ListBox()->CurrentItemIndex();
		TBool currentItemDeleted;
		if (currentItemIndex == pos)
			currentItemDeleted = ETrue;
		else
			currentItemDeleted = EFalse;

	//	AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
	//		iContainer->ListBox(), currentItemIndex, currentItemDeleted);

		itemArray->InsertL(pos,*text);
		iContainer->ListBox()->HandleItemAdditionL();

		CleanupStack::PopAndDestroy(); //text
	}
}
void CSymellaConnectionsView::RemoveAddressL(const TInetAddr& aAddress)
{
	//find address
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());
	TBuf<40> ips;
	TInt pos=-1;
	aAddress.Output(ips);
	for (TInt i=0; i<itemArray->Count(); i++)
	{
		if ((*itemArray)[i].Find(ips) != KErrNotFound)
			pos=i;
	}

	if (pos>-1)
	{
		itemArray->Delete(pos);

		TInt currentItemIndex = iContainer->ListBox()->CurrentItemIndex();
		TBool currentItemDeleted;
		if (currentItemIndex == pos)
			currentItemDeleted = ETrue;
		else
			currentItemDeleted = EFalse;

	//	AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
	//		iContainer->ListBox(), currentItemIndex, currentItemDeleted);

	//	iContainer->ListBox()->HandleItemAdditionL();
	}
}

//hostcache handling functions
void CSymellaConnectionsView::SetUltrapeersCountL(TInt aValue)
{
	iUltrapeerCount = aValue;
	SetHostCacheTextL();
}
void CSymellaConnectionsView::SetLeafsCountL(TInt aValue)
{
	iLeafCount = aValue;
	SetHostCacheTextL();
}
void CSymellaConnectionsView::SetUnidentifiedNodesCountL(TInt aValue)
{
	iUnidentifiedNodesCount = aValue;
	SetHostCacheTextL();
}
void CSymellaConnectionsView::SetWorkingNodesCountL(TInt aValue)
{
	iWorkingNodesCount = aValue;
	SetHostCacheTextL();
}

void CSymellaConnectionsView::SetCountsL(TInt aUltrapeers,TInt aLeafs,TInt aUnidentifieds,TInt aWorkings)
{
	iUltrapeerCount = aUltrapeers;
	iLeafCount = aLeafs;
	iUnidentifiedNodesCount = aUnidentifieds;
	iWorkingNodesCount = aWorkings;
	SetHostCacheTextL();
}

void CSymellaConnectionsView::SetHostCacheTextL()
{
	//last item of the list
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	HBufC* text = HBufC::NewLC(40);
	TPtr des = text->Des();

	des.Format(_L("Cache\tU:%d,W:%d,X:%d"),iUltrapeerCount,iWorkingNodesCount,iUnidentifiedNodesCount);

	TInt aIndex = itemArray->Count()-1;
	if (aIndex>-1) itemArray->Delete(aIndex);

	TInt currentItemIndex = iContainer->ListBox()->CurrentItemIndex();
	TBool currentItemDeleted;
	if (currentItemIndex == aIndex)
		currentItemDeleted = ETrue;
	else
		currentItemDeleted = EFalse;

//	AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
//		iContainer->ListBox(), currentItemIndex, currentItemDeleted);

	itemArray->AppendL(*text);
	iContainer->ListBox()->HandleItemAdditionL();

	CleanupStack::PopAndDestroy(); //text;
}

//webcache handling functions
void CSymellaConnectionsView::InsertWebCacheL(TInt aWebcacheIndex, TSAWebCacheRequestType aType)
{
	iWebcacheIndex = aWebcacheIndex;
	iWebCacheRequestType = aType;
	iIndicatorState = 0;
	//add the address to the start of the list
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	HBufC* text = HBufC::NewLC(64);
	TPtr des = text->Des();
	if (iWebCacheRequestType == EHostFileRequest)
		des.Format(_L("Conn\tWebCache[%d] IPs:0"), iWebcacheIndex);
	else
		des.Format(_L("Conn\tWebCache[%d] URL"), iWebcacheIndex);

	itemArray->InsertL(0, *text);
	iContainer->ListBox()->HandleItemAdditionL();

	CleanupStack::PopAndDestroy(); //text
}

void CSymellaConnectionsView::ModifyWebCacheL(TSAConnectionState aStatus)
{
	//add the address to the start of the list
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	//set text, and replace first item's text with it
	HBufC* text = HBufC::NewLC(64);
	TPtr des = text->Des();
	
	if (iWebCacheRequestType == EHostFileRequest) // node address request
	{
		if (aStatus == MSAConnectionObserver::EConnected) des.Format(_L("OK\tWCache[%d] IPs:%d"), iWebcacheIndex, iWebCacheIPCount);
		else if (aStatus == MSAConnectionObserver::EConnecting)
		{
			des.Format(_L("%c Conn\tWCache[%d] IPs:%d"), 
				KProgressIndicator()[iIndicatorState], iWebcacheIndex, iWebCacheIPCount);
				
			iIndicatorState++;
			
			if (iIndicatorState >= KProgressIndicator().Length())
				iIndicatorState = 0;
		}
		else if (aStatus == MSAConnectionObserver::EHandshaking) des.Format(_L("NetCon\tWCache[%d] IPs:%d"), iWebcacheIndex, iWebCacheIPCount);
		else if (aStatus == MSAConnectionObserver::EDisconnected) des.Format(_L("Disc\tWCache[%d] IPs:%d"), iWebcacheIndex, iWebCacheIPCount);
		else if (aStatus == MSAConnectionObserver::EClose) des.Format(_L("Close\tWCache[%d] IPs:%d"), iWebcacheIndex, iWebCacheIPCount);
		else if (aStatus == MSAConnectionObserver::EError) des.Format(_L("Error\tWCache[%d] IPs:%d"), iWebcacheIndex, iWebCacheIPCount);
		else des.Format(_L("Resolv\tWCache[%d] IPs:%d"), iWebcacheIndex, iWebCacheIPCount);	
	}
	else // webcache address request
	{
		if (aStatus == MSAConnectionObserver::EConnected) des.Format(_L("OK\tWCache[%d] URL"), iWebcacheIndex);
		else if (aStatus == MSAConnectionObserver::EConnecting)
		{
			des.Format(_L("%c Conn\tWCache[%d] URL"), 
				KProgressIndicator()[iIndicatorState], iWebcacheIndex);
				
			iIndicatorState++;
			
			if (iIndicatorState >= KProgressIndicator().Length())
				iIndicatorState = 0;
		}
		else if (aStatus == MSAConnectionObserver::EHandshaking) des.Format(_L("NetCon\tWCache[%d] URL"), iWebcacheIndex);
		else if (aStatus == MSAConnectionObserver::EDisconnected) des.Format(_L("Disc\tWCache[%d] URL"), iWebcacheIndex);
		else if (aStatus == MSAConnectionObserver::EClose) des.Format(_L("Close\tWCache[%d] URL"), iWebcacheIndex);
		else if (aStatus == MSAConnectionObserver::EError) des.Format(_L("Error\tWCache[%d] URL"), iWebcacheIndex);
		else des.Format(_L("Resolv\tWCache[%d] URL"), iWebcacheIndex);
	}
	

	itemArray->Delete(0);

	TInt currentItemIndex = iContainer->ListBox()->CurrentItemIndex();
	TBool currentItemDeleted;
	if (currentItemIndex == 0)
		currentItemDeleted = ETrue;
	else
		currentItemDeleted = EFalse;

//	AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
//		iContainer->ListBox(), currentItemIndex, currentItemDeleted);

	itemArray->InsertL(0,*text);
	iContainer->ListBox()->HandleItemAdditionL();

	CleanupStack::PopAndDestroy(); //text
}

void CSymellaConnectionsView::SetWebCacheAddressCountL(TInt aCount)
{
	iWebCacheIPCount = aCount;
	ModifyWebCacheL(MSAConnectionObserver::EConnected);
}

void CSymellaConnectionsView::RemoveWebCacheL()
{
	//add the address to the start of the list
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	itemArray->Delete(0);

	TInt currentItemIndex = iContainer->ListBox()->CurrentItemIndex();
	TBool currentItemDeleted;
	if (currentItemIndex == 0)
		currentItemDeleted = ETrue;
	else
		currentItemDeleted = EFalse;
	
	iWebCacheIPCount = 0;

//	AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
//		iContainer->ListBox(), currentItemIndex, currentItemDeleted);

//	iContainer->ListBox()->HandleItemAdditionL();
}

// End of File


