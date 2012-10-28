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
*  Name     : CSymellaSettingsContainer from SymellaSettingsContainer.cpp
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include "SymellaSettingsContainer.h"

#include <aknlists.h>
#include <barsread.h>
#include <Symella.rsg>
#include "Symella.hrh"
#include "FileSelectionSettingItem.h"
#include "AccessPointSettingItem.h"
#include "SAControl.h"
#include "DummySettingItem.h"
#include "SymellaAppUi.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaSettingssContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymellaSettingsContainer::ConstructL(const TRect& aRect)
{
    CreateWindowL();
	ConstructFromResourceL(R_SYMELLA_SETTING_ITEM_LIST);

	SetRect(aRect);
	ActivateL();
}


void CSymellaSettingsContainer::GetPreferencesL()
{
	PREFERENCES->ForceCreate();
}


CAknSettingItem* CSymellaSettingsContainer::CreateSettingItemL(TInt aIdentifier)
{
	iPort=PREFERENCES->GetPort();
	iDestDir.Copy(PREFERENCES->GetDestDir());
	iMaxStoredHits=PREFERENCES->GetMaxStoredHits();
	iLowestNondeletableHitCount=PREFERENCES->GetLowestNonDeletableHitCount();
	iUseGWC=PREFERENCES->GetUseGWC();
	iEnableConnectionProfiling=PREFERENCES->GetEnableConnectionProfiling();
	HBufC* temp = PREFERENCES->GetWebCacheUrls();
	iGWebCaches.Copy(*temp);
	delete temp;

	CAknSettingItem* settingItem = NULL;

	switch (aIdentifier)
	{
	case ESymellaSettingPort:
		{
			settingItem = new (ELeave) CAknIntegerEdwinSettingItem(aIdentifier,iPort);
		}
		break;
		
	case ESymellaSettingNetworkConnection:
	{
		iApId = TInt32(PREFERENCES->AccessPointId());
        iAccesPointSettingItem = new (ELeave) CAccessPointSettingItem(
            aIdentifier, iApId, iApName);
            
        settingItem = iAccesPointSettingItem;
	}
	break;
	
	case ESymellaSettingWebcaches:
		{
			iWebcacheText.Num(PREFERENCES->WebCacheCount());
			
			if (PREFERENCES->WebCacheCount() == 1)
				iWebcacheText.Append(_L(" webcache [edit]"));
			else
				iWebcacheText.Append(_L(" webcaches [edit]"));
			
			//iWebcacheText = _L("");
			settingItem = new (ELeave) CDummySettingItem(aIdentifier, iWebcacheText);
			settingItem->SetEmptyItemTextL(_L(""));
		}
		break;
		
	case ESymellaSettingDestDir:
		{
			iFileSelectionSettingItem = new (ELeave) CFileSelectionSettingItem(
            	aIdentifier, iDestDir, ECFDDialogTypeSave);
            
        	settingItem = iFileSelectionSettingItem;
		}
		break;
		
	case ESymellaSettingMaxStoredHits:
		{
			settingItem = new (ELeave) CAknIntegerEdwinSettingItem(aIdentifier,iMaxStoredHits);
		}
		break;
		
	case ESymellaSettingLowestNonDeletableHitCount:
		{
			settingItem = new (ELeave) CAknIntegerEdwinSettingItem(aIdentifier,iLowestNondeletableHitCount);
		}
		break;
		
	case ESymellaSettingUseGWC:
		{
			settingItem = new (ELeave) CAknBinaryPopupSettingItem(aIdentifier,iUseGWC);
		}
		break;
		
	case ESymellaSettingEnableConnectionProfiling:
		{
			settingItem = new (ELeave) CAknBinaryPopupSettingItem(aIdentifier,iEnableConnectionProfiling);
		}
		break;
		
	default:
		break;
	}
	return settingItem;
}

void CSymellaSettingsContainer::EditItemL(TInt aIndex, TBool aCalledFromMenu)
{
	CAknSettingItemList::EditItemL(aIndex,aCalledFromMenu);
	(*SettingItemArray())[aIndex]->StoreL();

	//save the new value to the database
	switch (aIndex)
	{
	/*case 0:
		{
			PREFERENCES->SetPort(iPort);
		}
		break;*/

	case 0:
		{			
			PREFERENCES->SetDestDir(iFileSelectionSettingItem->FileName());
			HandleChangeInItemArrayOrVisibilityL();			
		}
		break;
	case 1:
		{			
			PREFERENCES->SetAccessPointL(iApName, iApId);
			CTR->NetMgr()->SetNetworkConnectionId(iApId);					
			HandleChangeInItemArrayOrVisibilityL(); 			
		}
		break;
	case 2:
		{
			((CSymellaAppUi*)CEikonEnv::Static()->AppUi())->ActivateLocalViewL(KWebCacheViewId);				
		}
		break;
	case 3:
		{
			PREFERENCES->SetMaxStoredHits(iMaxStoredHits);
		}
		break;
	case 4:
		{
			PREFERENCES->SetLowestNonDeletableHitCount(iLowestNondeletableHitCount);
		}
		break;
	/*case 4:
		{
			PREFERENCES->SetUseGWC(iUseGWC);
		}
		break;
	case 3:
		{
			PREFERENCES->SetGWebCacheUrlsL(iGWebCaches);
		}
		break;*/
	case 5:
		{
			PREFERENCES->SetEnableConnectionProfiling(iEnableConnectionProfiling);
		}
		break;		
	default:
		break;
	}
}