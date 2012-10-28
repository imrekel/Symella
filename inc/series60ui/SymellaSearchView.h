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
*  Description:
*     Declares view for application.
* ============================================================================
*/

#ifndef SYMELLASEARCHVIEW_H
#define SYMELLASEARCHVIEW_H

// INCLUDES
#include <aknview.h>
#include "SADownloadManager.h"
#include "SymellaSearchContainer.h"

// FORWARD DECLARATIONS
class CSymellaSearchContainer;
class CEikMenuPane;

// CLASS DECLARATION

/**
*  CSymellaSearchView view class.
* 
*/
class CSymellaSearchView : public CAknView, public MSASearchResultsObserver
    {
    public: // Constructors and destructor

        /**
        * EPOC default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CSymellaSearchView();

    public: // Functions from base classes
        
        /**
        * From ?base_class ?member_description
        */
        TUid Id() const;

        /**
        * From ?base_class ?member_description
        */
        void HandleCommandL(TInt aCommand);

        /**
        * From ?base_class ?member_description
        */
        void HandleViewRectChange();

		void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

	public: // from MSASearchResultsObserver
	
		void SearchStringChangedL(const TDesC8& aNewSearchString);

		void InsertHitL(TInt aIndex, 
						const TDesC8& aFileName, 
						TUint32 aFileSize, 
						TInt aHitCount,
						TBool aRefreshList = ETrue);

		void ModifyHitCountL(TInt aIndex, TInt aHitCount);

		void RemoveHitL(TInt aIndex);

		void ResetHitsL();
		
	private:
	
		inline TInt CurrentItemIndex();

		void CloseFindBoxL();
		
		void UpdateFilterL();

    private:

        /**
        * From AknView, ?member_description
        */
        void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
            const TDesC8& aCustomMessage);

        /**
        * From AknView, ?member_description
        */
        void DoDeactivate();

    private: // Data
        CSymellaSearchContainer* iContainer;
    };
   
    
inline TInt CSymellaSearchView::CurrentItemIndex() {
	return iContainer->CurrentItemIndex();
}

#endif

// End of File
