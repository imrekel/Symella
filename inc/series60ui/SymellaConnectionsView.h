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
*  Name     : CSymellaConnectionsView from SymellaView3.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
*  Description:
*     Declares view for application.
* ============================================================================
*/

#ifndef SYMELLACONNECTIONSVIEW_H
#define SYMELLACONNECTIONSVIEW_H

// INCLUDES
#include <aknview.h>
#include "SAControl.h"

// FORWARD DECLARATIONS
class CSymellaConnectionsContainer;
class CEikMenuPane;

// CLASS DECLARATION

/**
*  CSymellaTransfersView view class.
* 
*/
class CSymellaConnectionsView : public CAknView, public MSAConnectionObserver
    {
    public: // Constructors and destructor

        /**
        * EPOC default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CSymellaConnectionsView();

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

	public: // from MSAConnectionObserver
		void AddAddressL(const TInetAddr& aAddress, const TBool aIsSemPeer);
		void SetUltrapeerL(const TInetAddr& aAddress);
		void SetStateL(const TInetAddr& aAddress, TSAConnectionState aState);
		void RemoveAddressL(const TInetAddr& aAddress);

		//hostcache handling functions
		void SetUltrapeersCountL(TInt aValue);
		void SetLeafsCountL(TInt aValue);
		void SetUnidentifiedNodesCountL(TInt aValue);
		void SetWorkingNodesCountL(TInt aValue);
		void SetCountsL(TInt aUltrapeers,TInt aLeafs,TInt aUnidentifieds,TInt aWorkings);

		void InsertWebCacheL(TInt aWebcacheIndex, TSAWebCacheRequestType aType);
		void ModifyWebCacheL(TSAConnectionState aStatus);
		void SetWebCacheAddressCountL(TInt aCount);
		void RemoveWebCacheL();

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

		void SetHostCacheTextL();

    private: // Data
        CSymellaConnectionsContainer* iContainer;

		TInt iUltrapeerCount;
		TInt iLeafCount;
		TInt iUnidentifiedNodesCount;
		TInt iWorkingNodesCount;

		TInt iWebCacheIPCount;
		
		TInt iWebcacheIndex;
		
		TInt iIndicatorState;
		
		TSAWebCacheRequestType iWebCacheRequestType;
    };

#endif

// End of File
