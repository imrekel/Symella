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
*  Name     : CSymellaConsoleView from SymellaConsoleView.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
*  Description:
*     Declares a view containing a console for debug/trace information.
* ============================================================================
*/

#ifndef SYMELLACONSOLEVIEW_H
#define SYMELLACONSOLEVIEW_H

// INCLUDES
#include <aknview.h>
#include "salogger.h"

// FORWARD DECLARATIONS
class CSymellaConsoleContainer;
class CPlainText;

// CLASS DECLARATION
const TInt KMaxConsoleBufferSize = 512; // 

/**
*  CSymellaConsoleView view class.
* 
*/
class CSymellaConsoleView : public CAknView, public MLogObserver
{
    public: // Constructors and destructor

        /**
        * EPOC default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CSymellaConsoleView();

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

	public:

		CPlainText* TextBuffer();

	protected:  // from MLogObserver

		void HandleLogWriteL(const TDesC& aText);

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
        CSymellaConsoleContainer* iContainer;

		/**
		 * The text shown on the console
		 */
		CPlainText* iTextBuffer;
};


inline CPlainText* CSymellaConsoleView::TextBuffer() {
	return iTextBuffer;
}

#endif

// End of File
