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
*  Name     : CSymellaSearchView from SymellaView2.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <Symella.rsg>
#include "SymellaSearchView.h"
#include "SymellaSearchContainer.h" 
#include "SAControl.h"
#include "SADownloadManager.h"
#include "SADownloadCandidate.h"
#include "SymellaAppUi.h"
#include <aknlists.h>
#include <eikenv.h>
#include <aknappui.h>
#include <eikmenup.h>
#include <AknQueryDialog.h>
#include <aknnotewrappers.h>
#include "symella.hrh"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaSearchView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymellaSearchView::ConstructL()
{
    BaseConstructL( R_SYMELLA_SEARCHVIEW );

    iContainer = new (ELeave) CSymellaSearchContainer;
    iContainer->SetMopParent(this);
    iContainer->ConstructL( ClientRect() );
	iContainer->MakeVisible(EFalse);
	iContainer->SetRect(ClientRect());
}

// ---------------------------------------------------------
// CSymellaSearchView::~CSymellaSearchView()
// ?implementation_description
// ---------------------------------------------------------
//
CSymellaSearchView::~CSymellaSearchView()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    }

// ---------------------------------------------------------
// TUid CSymellaSearchView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CSymellaSearchView::Id() const
    {
    return KSearchViewId;
    }

// ---------------------------------------------------------
// CSymellaSearchView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaSearchView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
    {
		case ESymellaCmdFileNameOrder:
		{
			CloseFindBoxL();
			CTR->DlManager()->ChangeHitsOrderingL(EByFileName);
			iContainer->DrawDeferred();
		}
		break;

		case ESymellaCmdAddressCountOrder:
		{
			CloseFindBoxL();
			CTR->DlManager()->ChangeHitsOrderingL(EByAddressCount);
			iContainer->DrawDeferred();
		}
		break;

		case ESymellaCmdNoOrdering:
		{
			CloseFindBoxL();
			CTR->DlManager()->ChangeHitsOrderingL(EByArrival);
			iContainer->DrawDeferred();
		}
		break;
		
		case EAknCmdExit:
    	case EEikCmdExit:
    	{
    		CloseFindBoxL();
    		AppUi()->HandleCommandL(EEikCmdExit); 
    	}
    	break;

    	
		case EAknSoftkeyExit:
		case EAknSoftkeyBack:
        {
        	CloseFindBoxL();
            AppUi()->HandleCommandL(EAknSoftkeyExit);        
        }
		break;

		// starting a new download
		case ESymellaCmdDownload:
		{
			TInt index = CurrentItemIndex();
			if (index >= 0)
			{

				TSADownloadAttemptResult res = 
					CTR->DlManager()->StartDownloadL(index, EFalse);
								
				if(res != ECannotGetFreeSpace) 
				{
					CSADownloadCandidate* dl = CTR->DlManager()->DownloadCandidate(index);
					CTR->Profiler()->ProfileDownloadL(dl->Name(), dl->Size());
				}
				
				switch (res)
				{					
					case ECannotCreateFile:
					{
						(new (ELeave) CAknWarningNote)->ExecuteLD(
							_L("Cannot create file!"));
					}
					break;

					case EAlreadyDownloading:
					{
						(new (ELeave) CAknWarningNote)->ExecuteLD(
							_L("The selected file is already being downloaded!"));
					}
					break;

					case ECannotAccessDirectory:
					{
						(new (ELeave) CAknWarningNote)->ExecuteLD(
							_L("Cannot access the specified download directory! (check settings)"));
					}
					break;

					case EInvalidPath:
					{
						(new (ELeave) CAknWarningNote)->ExecuteLD(
							_L("Invalid download path! (check settings)"));
					}
					break;

					case EFailure:
					{
						(new (ELeave) CAknWarningNote)->ExecuteLD(
							_L("Cannot start download! (general failure)"));
					}
					break;

					case ECannotGetFreeSpace:
					{
						(new (ELeave) CAknWarningNote)->ExecuteLD(
							_L("Download failure! (cannot calculate free space on target drive)"));						
					}
					break;

					case ENotEnoughFreeSpace:
					{
						(new (ELeave) CAknWarningNote)->ExecuteLD(
							_L("Not enough disk space on target drive!"));
							
						CTR->Profiler()->ProfileDiskfullL();
					}
					break;

					case ECannotOverwrite:
					{
						(new (ELeave) CAknWarningNote)->ExecuteLD(
							_L("Cannot overwrite the file!"));
					}
					break;

					case EFileExists:
					{
						// the file exists, asking to overwrite
						CAknQueryDialog* query = CAknQueryDialog::NewL();
						CleanupStack::PushL(query);
						_LIT(KOverwritePrompt, "File already exists. Overwrite?");
						query->SetPromptL(KOverwritePrompt);
						CleanupStack::Pop(); // query

						if (query->ExecuteLD(R_GENERAL_QUERY))
						{
							if (CTR->DlManager()->StartDownloadL(index, ETrue) == ECannotOverwrite)
							{
								(new (ELeave) CAknWarningNote)->ExecuteLD(
									_L("Cannot overwrite the file!"));
							}
						}
					}
					break;

					default:;
				}
			}
		}
		break;

        default:
        {
            AppUi()->HandleCommandL( aCommand );        
        }
		break;       
	}
}

// ---------------------------------------------------------
// CSymellaSearchView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSymellaSearchView::HandleViewRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CSymellaSearchView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaSearchView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{ 
	iContainer->SetRect( ClientRect() );
	iContainer->MakeVisible(ETrue);
	iContainer->FindBox()->ResetL();
	AppUi()->AddToStackL( *this, iContainer );
}

void CSymellaSearchView::CloseFindBoxL()
{
	if ( iContainer )
    {        
//		if (iFindBox)
		// Empty FindBox Search Text		
		_LIT(KBlank, "");
		iContainer->FindBox()->SetSearchTextL(KBlank);

		//iContainer->FindBox()->SetFocus(EFalse);
		// Press Backspace on Empty FindBox to Close it
		TBool refresh = false;
		TKeyEvent key;
		key.iCode = 8;
		key.iScanCode = 1;
		key.iRepeats = 1;
		key.iModifiers = 0;
		AknFind::HandleFindOfferKeyEventL(key, EEventKey, iContainer, iContainer->ListBox(), iContainer->FindBox(), ETrue, refresh);
	}
}

// ---------------------------------------------------------
// CSymellaSearchView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymellaSearchView::DoDeactivate()
{
	if ( iContainer )
    {
		AppUi()->RemoveFromViewStack( *this, iContainer );
		iContainer->MakeVisible(EFalse);		
    }    
}

void CSymellaSearchView::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
{
	switch (aResourceId)
	{
		case R_SYMELLA_SEARCHVIEW_MENU:	
		{
			if (CurrentItemIndex() < 0)
			{
				aMenuPane->SetItemDimmed(ESymellaCmdDownload, ETrue);
			}
			else
				aMenuPane->SetItemDimmed(ESymellaCmdDownload, EFalse);

		}
		break;

		default:
		break;
	}
	AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
}


void CSymellaSearchView::InsertHitL(TInt aIndex, 
									const TDesC8& aFileName, 
									TUint32 /*aFileSize*/, 
									TInt aHitCount,
									TBool aRefreshList)
{
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	HBufC* item = HBufC::NewMaxLC(aFileName.Length() + 3);
	TPtr ptr(item->Des());	
	ptr.Format(_L("%d\t"), aHitCount);
	ptr.SetLength(aFileName.Length() + 3);

	// searching for the beginning of the filename
	TInt i = 0;
	while (ptr[i] != '\t') i++;
	i++;

	TPtr fileNamePtr(const_cast<TUint16*>(ptr.Ptr()) + i, aFileName.Length());
	fileNamePtr.Copy(aFileName);
	
	itemArray->InsertL(aIndex, *item);
	
	CleanupStack::PopAndDestroy(); // item

	if (aRefreshList) 
	{
		iContainer->ListBox()->HandleItemAdditionL();
		CSymellaAppUi* appui=(CSymellaAppUi*)AppUi();
		appui->ReplaceSearchTextL(itemArray->Count());
		UpdateFilterL();
	}
}


void CSymellaSearchView::ModifyHitCountL(TInt aIndex, TInt aHitCount)
{
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	TPtrC oldItem = (*itemArray)[aIndex];

	HBufC* newItem = HBufC::NewMaxLC(oldItem.Length() + 1);
	
	// searching for the beginning of the filename
	TInt i = 0;
	while (oldItem[i] != '\t') i++;
	i++;

	TPtrC fileName = oldItem.Right(oldItem.Length() - i);
	TPtr ptr(newItem->Des());
	ptr.SetLength(0);
	ptr.Format(_L("%d\t"), aHitCount);
	ptr.Append(fileName);

	itemArray->Delete(aIndex);
	iContainer->ListBox()->HandleItemRemovalL();
	itemArray->InsertL(aIndex, *newItem);
	iContainer->ListBox()->HandleItemAdditionL();

	CleanupStack::PopAndDestroy(); // newItem*/
	UpdateFilterL();
}


void CSymellaSearchView::RemoveHitL(TInt aIndex)
{
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	TInt currentItemIndex = CurrentItemIndex();
	itemArray->Delete(aIndex);

	TBool currentItemDeleted;
	if (currentItemIndex == aIndex)
		currentItemDeleted = ETrue;
	else
		currentItemDeleted = EFalse;

	AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
		iContainer->ListBox(), currentItemIndex, currentItemDeleted);

	CSymellaAppUi* appui=(CSymellaAppUi*)AppUi();
	appui->ReplaceSearchTextL(itemArray->Count());
	UpdateFilterL();
}


void CSymellaSearchView::ResetHitsL()
{
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iContainer->ListBox()->Model()->ItemTextArray());

	itemArray->Reset();
	iContainer->ListBox()->HandleItemRemovalL();

	CSymellaAppUi* appui=(CSymellaAppUi*)AppUi();
	appui->ReplaceSearchTextL(-1);
}


void CSymellaSearchView::SearchStringChangedL(const TDesC8& aNewSearchString)
{
	if (iContainer)
	{
		iContainer->FindBox()->ResetL();
		if (aNewSearchString == KNullDesC8)
			iContainer->ListBox()->View()->SetListEmptyTextL(_L("(no active search)"));	
		else
		{
			HBufC* searchString = HBufC::NewLC(aNewSearchString.Length() + 50);
			TPtr searchStringPtr (searchString->Des());
			searchStringPtr.Copy(aNewSearchString);
			searchStringPtr.Insert(0, _L("(searching for \""));
			searchStringPtr.Append(_L("\")"));
			iContainer->ListBox()->View()->SetListEmptyTextL(searchStringPtr);
			CleanupStack::PopAndDestroy(); // searchString		
		}
	}	
}


void CSymellaSearchView::UpdateFilterL()
{
	if (iContainer->FindBox()->TextLength() > 0)
	{
		CAknFilteredTextListBoxModel* model =
            static_cast<CAknFilteredTextListBoxModel*>(iContainer->ListBox()->Model());
            
        CAknListBoxFilterItems* filter = model->Filter();
        filter->UpdateCachedDataL();
        filter->HandleItemArrayChangeL();
	}	
}



// End of File

