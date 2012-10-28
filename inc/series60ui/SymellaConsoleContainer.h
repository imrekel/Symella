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
*  Name     : CSymellaConsoleContainer from SymellaConsoleContainer.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
*  Description:
*     Declares container control for application.
* ============================================================================
*/

#ifndef SYMELLACONSOLECONTAINER_H
#define SYMELLACONSOLECONTAINER_H

// INCLUDES
#include <coecntrl.h>
   
// FORWARD DECLARATIONS
class CEikEdwin;
class CPlainText;
class CEikEdwin;

// CLASS DECLARATION

/**
*  CSymellaConsoleContainer  container control class.
*  
*/
class CSymellaConsoleContainer : public CCoeControl
    {
    public: // Constructors and destructor
        
        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect, CPlainText* aEdwinDocument);

        /**
        * Destructor.
        */
        ~CSymellaConsoleContainer();

	public:

		/**
		 * Handle change in text model
		 */
		void HandleTextChangeL();

		CEikEdwin* Edwin() {
			return iEdwin;
		}

	public: // from CCoeControl

		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

    private: // from CCoeControl
     
        void SizeChanged();
       
        TInt CountComponentControls() const;

        CCoeControl* ComponentControl(TInt aIndex) const;
        
    private: //data
        
		/**
		 * Displays the console data
		 */
        CEikEdwin* iEdwin;
    };

#endif

// End of File
