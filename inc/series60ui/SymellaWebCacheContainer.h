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

#ifndef SYMELLAWEBCACHECONTAINER_H__
#define SYMELLAWEBCACHECONTAINER_H__

// INCLUDES
#include <coecntrl.h>
#include <aknlists.h> 
#include "SymellaAppUi.h"
   
// FORWARD DECLARATIONS
class MAknsControlContext; // for skins support
class CSAPreferences;

// CLASS DECLARATION

/**
 *  CSymellaWebCacheContainer  container control class.
 */
class CSymellaWebCacheContainer : public CCoeControl, public MCoeControlObserver
    {
    public: // Constructors and destructor
        
        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container, aAppUi SymTorrentAppUi, aTorrentMgr the TorrentManager.
        */
        void ConstructL(const TRect& aRect, CSymellaAppUi* aAppUi);

        /**
        * Destructor.
        */
        ~CSymellaWebCacheContainer();

       /**
        * Returns the TorrentListBox
        */
		CAknSingleNumberStyleListBox* ListBox();
		
		void RefreshListL()	;

    private: // Functions from base classes
       /**
        * From CoeControl,SizeChanged.
        */
        void SizeChanged();

       /**
        * From CoeControl,CountComponentControls.
        */
        TInt CountComponentControls() const;

       /**
        * From CCoeControl,ComponentControl.
        */
        CCoeControl* ComponentControl(TInt aIndex) const;

       /**
        * From CCoeControl,Draw.
        */
        void Draw(const TRect& aRect) const;

		/**
		* Pass skin information if needed.
		*/
		TTypeUid::Ptr MopSupplyObject(TTypeUid aId);
       	
		/**
		* From MCoeControlObserver
		* Acts upon changes in the hosted control's state. 
		*
		* @param	aControl	The control changing its state
		* @param	aEventType	The type of control event 
		*/
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
        
		/**
		* Key events
		*/
		TKeyResponse OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType);

    private: //data    
        
        CAknSingleNumberStyleListBox*			iListBox; // files listbox
        
		MAknsControlContext*			iBackGround; // for skins support
		
		CSymellaAppUi*					iAppUi;
		
		CSAPreferences*					iPreferences;
		
		
    };

#endif

// End of File
