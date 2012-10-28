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
*  Name     : CSymellaTransfersView from SymellaView3.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
*  Description:
*     Declares view for application.
* ============================================================================
*/

#ifndef SYMELLATRANSFERSVIEW_H
#define SYMELLATRANSFERSVIEW_H

// INCLUDES
#include <aknview.h>
#include "SADownloadManager.h"


// FORWARD DECLARATIONS
class CSymellaTransfersContainer;
class CEikMenuPane;

// CLASS DECLARATION

/**
*  CSymellaTransfersView view class.
* 
*/
class CSymellaTransfersView : public CAknView, public MSADownloadResultsObserver
    {
    public: // Constructors and destructor

        /**
        * EPOC default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CSymellaTransfersView();

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

		void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

	public: //from MSADownloadResultsObserver

		void InsertDownloadL(	TInt aIndex, 
								const TDesC8& aFileName, 
								TUint aFullSize, 
								TInt aPeerCount,											
								TSADownloadStatus aStatus );

		void ModifyDownloadL(	TInt aIndex, 
								const TDesC8& aFileName, 
								TUint aFullSize, 
								TUint aDownloadedSize, 
								TInt aPeerCount, 
								TSADownloadStatus aStatus);

		void RemoveDownloadL(TInt aIndex);

    private:

		HBufC* CreateItemTextLC(const TDesC& aFileName, 
								TUint aSize, 
								TInt aPeerCount, 
								TInt aPercent, 
								TSADownloadStatus aStatus);

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
		TInt							iProgressIndicatorIndex;
        CSymellaTransfersContainer*		iContainer;
    };

#endif

// End of File
