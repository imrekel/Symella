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
*  Name     : CSettingsSymellaContainer from SymellaSettingsContainer.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
*  Description:
*     Declares container control for application.
* ============================================================================
*/

#ifndef SYMELLASETTINGSCONTAINER_H
#define SYMELLASETTINGSCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <aknsettingitemlist.h>
#include "SAPreferences.h"
 
// FORWARD DECLARATIONS
class CFileSelectionSettingItem;
class CAccessPointSettingItem;

// CLASS DECLARATION

/**
 * CSymellaSettingsContainer 
 *
 * Container window for the Settings view
 */
class CSymellaSettingsContainer : public CAknSettingItemList
    {
    public: // Constructors and destructor
        
        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect);
		//TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,	TEventCode aType);
		CAknSettingItem * CreateSettingItemL( TInt aIdentifier );

		//set an item
		void EditItemL(TInt aIndex, TBool aCalledFromMenu);

		void GetPreferencesL();

        /**
        * Destructor.
        */
        //~CSymellaSettingsContainer();

    private: // from CCoeControl
		TBuf<1000>	iGWebCaches;
		TFileName	iDestDir;
		TInt		iPort;
		TInt		iMaxStoredHits;
		TInt		iLowestNondeletableHitCount;
		TBool		iUseGWC;
		TBool iEnableConnectionProfiling;
		
		TInt32						iApId;
		TBuf<128>					iApName;
		
		CFileSelectionSettingItem* iFileSelectionSettingItem;
		
		CAccessPointSettingItem*	iAccesPointSettingItem;
		
		TBuf<64>	iWebcacheText;
    };

#endif

// End of File
