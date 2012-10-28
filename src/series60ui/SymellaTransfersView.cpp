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
*  Name     : CSymellaTransfersView from SymellaView2.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include  <aknviewappui.h>
#include  <avkon.hrh>
#include  <Symella.rsg>
#include  "SymellaTransfersView.h"
#include  "SymellaTransfersContainer.h"
#include "SAControl.h"
#include "SADownloadManager.h"
#include "SymellaAppUi.h"
#include <aknlists.h>
#include <eikenv.h>
#include <aknappui.h>
#include <eikmenup.h>
#include <DocumentHandler.h>
#include <apmstd.h>
#include <AknQueryDialog.h>
#include "symella.hrh"
#include "SAPreferences.h"
#include "SAStringFormatter.h"

_LIT(KProgressIndicator, "|/-\\|/-\\");

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaTransfersView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymellaTransfersView::ConstructL()
{
    BaseConstructL( R_SYMELLA_TRANSFERSVIEW );

	iContainer = new (ELeave) CSymellaTransfersContainer;
    iContainer->SetMopParent(this);
    iContainer->ConstructL(ClientRect(), this);
	iContainer->MakeVisible(EFalse);
	iContainer->SetRect(ClientRect());
}

// ---------------------------------------------------------
// CSymellaTransfersView::~CSymellaTransfersView()
// ?implementation_description
// ---------------------------------------------------------
//
CSymellaTransfersView::~CSymellaTransfersView()
{
    if ( iContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iContainer );
    }

    delete iContainer;
}

// ---------------------------------------------------------
// TUid CSymellaTransfersView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CSymellaTransfersView::Id() const
{
    return KTransferViewId;
}

// ---------------------------------------------------------
// CSymellaTransfersView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaTransfersView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
    {
        case ESymellaCmdRemoveDownload:
        {
			TInt index = iContainer->ListBox()->CurrentItemIndex();
			if (index >= 0)
			{
				if (    (CTR->DlManager()->GetDownloadState(index) != CSADownload::EComplete)
					 && (CTR->DlManager()->IsDownloadStarted(index)) )
				{
					// ask to delete the partially downloaded file					
					CAknQueryDialog* query = CAknQueryDialog::NewL();
					CleanupStack::PushL(query);
					_LIT(KPrompt, "Are you sure you want to cancel download?");
					query->SetPromptL(KPrompt);
					CleanupStack::Pop(); // query
										
					if (query->ExecuteLD(R_GENERAL_QUERY))
					{
						CAknQueryDialog* query2 = CAknQueryDialog::NewL();
						CleanupStack::PushL(query2);
						_LIT(KPrompt2, "Delete the partially downloaded file?");
						query2->SetPromptL(KPrompt2);
						CleanupStack::Pop(); // query2

						if (query2->ExecuteLD(R_GENERAL_QUERY))
						{
							CTR->DlManager()->RemoveDownloadL(index, ETrue);
							break;
						}
						else
							CTR->DlManager()->RemoveDownloadL(index, EFalse);	
					}							
				}
				else													
					CTR->DlManager()->RemoveDownloadL(index, EFalse);
			}            
            break;
        }
		case ESymellaCmdOpenWithApp:
		{
			TInt index = iContainer->ListBox()->CurrentItemIndex();
			if (index >= 0)
			{
				CDocumentHandler* handler = CDocumentHandler::NewL(NULL);
				CleanupStack::PushL(handler);
				TFileName descr;

				//get file path
				descr.Append(CTR->DlManager()->GetDownloadFileName(index));
				/*descr.Append(_L("\\"));
				HBufC* bla = HBufC::NewLC(CTR->DlManager()->GetSelectedDownloadName(iContainer->ListBox()->CurrentItemIndex()).Length());
				TPtr des = bla->Des();
				des.Copy(CTR->DlManager()->GetSelectedDownloadName(iContainer->ListBox()->CurrentItemIndex()));
				descr.Append(des);*/

				TDataType nullType;
				// Launch the appropriate application for this file
				handler->OpenFileL(descr, nullType);
				//CleanupStack::PopAndDestroy(); //bla
				CleanupStack::PopAndDestroy(); // handler
			}

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

// ---------------------------------------------------------
// CSymellaTransfersView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSymellaTransfersView::HandleViewRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CSymellaTransfersView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaTransfersView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
	iContainer->SetRect( ClientRect() );
	iContainer->MakeVisible(ETrue);
    AppUi()->AddToStackL( *this, iContainer );
}

// ---------------------------------------------------------
// CSymellaTransfersView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaTransfersView::DoDeactivate()
{
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
		iContainer->MakeVisible(EFalse);
        }
    
    //delete iContainer;
    //iContainer = NULL;
}


void CSymellaTransfersView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
{
	switch (aResourceId)
	{
		case R_SYMELLA_TRANSFERSVIEW_MENU:	
		{
			TInt index = iContainer->ListBox()->CurrentItemIndex();
			if (index < 0)
			{
				aMenuPane->SetItemDimmed(ESymellaCmdRemoveDownload, ETrue);
			
			}
			else
			{
				aMenuPane->SetItemDimmed(ESymellaCmdRemoveDownload, EFalse);
				
			}

			if ((index >= 0) && (CTR->DlManager()->GetDownloadState(index) == CSADownload::EComplete))
			{
				aMenuPane->SetItemDimmed(ESymellaCmdOpenWithApp, EFalse);
			}
			else
			{
				aMenuPane->SetItemDimmed(ESymellaCmdOpenWithApp, ETrue);
			}			
		}
		break;

		default:
		break;
	}
	AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);	
}

void CSymellaTransfersView::InsertDownloadL(TInt aIndex, 
											const TDesC8& aFileName, 
											TUint aFullSize, 
											TInt aPeerCount,											
											TSADownloadStatus aStatus)
{
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	HBufC* fileName16 = HBufC::NewLC(aFileName.Length());
	{
		TPtr ptr(fileName16->Des());
		ptr.Copy(aFileName);
	}

	HBufC* item = CreateItemTextLC(*fileName16, aFullSize, aPeerCount, 0, aStatus);
	itemArray->InsertL(aIndex, *item);
	iContainer->ListBox()->HandleItemAdditionL();
	CleanupStack::PopAndDestroy(); // item
	CleanupStack::PopAndDestroy(); // fileName16
}

void CSymellaTransfersView::ModifyDownloadL(TInt aIndex, 
											const TDesC8& aFileName, 
											TUint aFullSize, 
											TUint aDownloadedSize, 
											TInt aPeerCount, 
											TSADownloadStatus aStatus)
{
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	HBufC* fileName16 = HBufC::NewLC(aFileName.Length());
	{
		TPtr ptr(fileName16->Des());
		ptr.Copy(aFileName);
	}

	if (aStatus == EDownloadInProgress)
	{
		iProgressIndicatorIndex++;
		
		if (iProgressIndicatorIndex > (TPtrC16(KProgressIndicator).Length()-1) )
			iProgressIndicatorIndex = 0;
	}

	TInt percent = TInt(TReal(aDownloadedSize) / (TReal(aFullSize) / 100.0));
	HBufC* newItem = CreateItemTextLC(*fileName16, aFullSize, 
		aPeerCount, percent, aStatus);
	
	itemArray->Delete(aIndex);
	//iContainer->ListBox()->HandleItemRemovalL();

	TInt currentItemIndex = iContainer->ListBox()->CurrentItemIndex();
	TBool currentItemDeleted;
	if (currentItemIndex == aIndex)
		currentItemDeleted = ETrue;
	else
		currentItemDeleted = EFalse;

	/*AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
		iContainer->ListBox(), currentItemIndex, currentItemDeleted);*/

	itemArray->InsertL(aIndex, *newItem);
	iContainer->ListBox()->HandleItemAdditionL();

	CleanupStack::PopAndDestroy(); // newItem
	CleanupStack::PopAndDestroy(); // fileName16
}

void CSymellaTransfersView::RemoveDownloadL(TInt aIndex)
{
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	TInt currentItemIndex = iContainer->ListBox()->CurrentItemIndex();
	itemArray->Delete(aIndex);

	TBool currentItemDeleted;
	if (currentItemIndex == aIndex)
		currentItemDeleted = ETrue;
	else
		currentItemDeleted = EFalse;

	AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
		iContainer->ListBox(), currentItemIndex, currentItemDeleted);

	//CSymellaAppUi* appui=(CSymellaAppUi*)AppUi();
	//appui->ReplaceSearchTextL(itemArray->Count());
}

HBufC* CSymellaTransfersView::CreateItemTextLC(const TDesC& aFileName, 
											   TUint aSize, 
											   TInt aPeerCount, 
											   TInt aPercent, 
											   TSADownloadStatus aStatus)
{
	HBufC* item = HBufC::NewMaxLC(aFileName.Length() + 42);
	TPtr ptr(item->Des());

	TBuf<50> fileSize;
	TSAStringFormatter::AppendFileLength(aSize, fileSize);
	
	TReal mbs;
	mbs = (TReal)(aSize) / ((TReal)1048576);

	if (aStatus == EDownloadQueued)
	{
		ptr.Format(_L("\t%S\t%S %d%%, QUEUED"), &aFileName, &fileSize, aPercent);
	}
	else

	if (aStatus == EDownloadFailed)
	{
		ptr.Format(_L("\t%S\t%S %d%%, FAILED"), &aFileName, &fileSize, aPercent);
	}
	else
	{		
		if (aPercent == 100)
		{
			ptr.Format(_L("\t%S\t%S, COMPLETE"), &aFileName, &fileSize);
		}
		else
		{
			if (aPeerCount == 0)
				ptr.Format(_L("\t%S\t%S, trying, %d%%"), &aFileName, &fileSize, aPercent);
			else if (aPeerCount == 1)
				ptr.Format(_L("\t%S\t%S, 1 thread, %d%%"), &aFileName, &fileSize, aPercent);
			else
				ptr.Format(_L("\t%S\t%S, %d threads, %d%%"), &aFileName, &fileSize, aPeerCount, aPercent);

			if (aStatus == EDownloadInProgress)
			{
				TBuf<4> indicator;
				indicator.Format(_L(" %c"), TPtrC16(KProgressIndicator)[iProgressIndicatorIndex]);
				ptr.Append(indicator);
			}
		}
	}

	return item;
}



// End of File

