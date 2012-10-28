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
*  Name     : CSymellaAppUi from SymellaAppUi.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
*  Description:
*     Declares UI class for application.
* ============================================================================
*/

// uncomment the next line if to want to have a console view in the 
// application, you have to uncomment the same line at beginning of Symella.rss too!
//#define CONSOLE

#ifndef SYMELLAAPPUI_H
#define SYMELLAAPPUI_H

// INCLUDES
#include <eikapp.h>
#include <eikdoc.h>
#include <e32std.h>
#include <coeccntx.h>
#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include "NetworkManager.h"

// FORWARD DECLARATIONS
class CSymellaContainer;
class CPlainText;

// CONSTS
const TUid KConnectionsViewId = {1};
const TUid KSearchViewId = {2};
const TUid KTransferViewId = {3};
const TUid KConsoleViewId = {4};
const TUid KSettingsViewId = {5};
const TUid KWebCacheViewId = {6};

/**
 * Application UI class.
 * Provides support for the following features:
 * - EIKON control architecture
 * - view architecture
 * - status pane
 */
class CSymellaAppUi : public CAknViewAppUi, public MAccessPointSupplier
    {
    public: // // Constructors and destructor

        /**
        * EPOC default constructor.
        */      
        void ConstructL();

        /**
        * Destructor.
        */      
        ~CSymellaAppUi();
        
    public: // New functions

		void HideTabGroup();

		void ShowTabGroupL();

		void ReplaceSearchTextL(TInt aCounter);

    public: // Functions from base classes

    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);
        
    private: // from MAccessPointSupplier
    
    	virtual TBool GetIapIdL(TInt32& aAccesPointId, TDes& aAccesPointName);

    private:
        /**
        * From CEikAppUi, takes care of command handling.
        * @param aCommand command to be handled
        */
        void HandleCommandL(TInt aCommand);

        /**
        * From CEikAppUi, handles key events.
        * @param aKeyEvent Event to handled.
        * @param aType Type of the key event. 
        * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed). 
        */
        TKeyResponse HandleKeyEventL(
            const TKeyEvent& aKeyEvent,TEventCode aType);
        
        void HandleResourceChangeL(TInt aType);

    private: //Data
    
        CAknNavigationControlContainer* iNaviPane;
        
        CAknTabGroup*                   iTabGroup;
        
        CAknNavigationDecorator*        iDecoratedTabGroup;		
    };

#endif

// End of File
