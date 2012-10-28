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

#ifndef SYMELLAWEBCACHEVIEW_H__
#define SYMELLAWEBCACHEVIEW_H__

// INCLUDES
#include <aknview.h>
#include <eikmenup.h>
#include "SymellaAppUi.h"
#include "SAPreferences.h"

// FORWARD DECLARATIONS
class CSymellaWebCacheContainer;

// CLASS DECLARATION

/**
*  CSymellaWebCacheView view class.
* 
*/
class CSymellaWebCacheView : public CAknView, public MSAWebCacheListObserver
{
public: // Constructors and destructor

    /**
    * EPOC default constructor.
    */
    void ConstructL(CSymellaAppUi* aAppUi);

    /**
    * Destructor.
    */
    ~CSymellaWebCacheView();

public: // Functions from base classes
    
    /**
    * From CAknView returns Uid of View
    * @return TUid uid of the view
    */
    TUid Id() const;

    /**
    * From MEikMenuObserver delegate commands from the menu
    * @param aCommand a command emitted by the menu 
    * @return void
    */
    void HandleCommandL(TInt aCommand);

    /**
    * From CAknView reaction if size change
    * @return void
    */
    void HandleViewRectChange();
    
public: // New functions
    /**
    * Inserts a file into the list
    */
	void InsertFileFromTorrent(TBuf<300> aData);
	
private:

    void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
        const TDesC8& aCustomMessage);

    void DoDeactivate();
    
    void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);
    
private: // from MSAWebCacheListObserver

		virtual void WebCacheListChangedL();

private: // Data

    CSymellaWebCacheContainer*	iContainer;
    
	CSymellaAppUi*				iAppUi;
};

#endif

// End of File
