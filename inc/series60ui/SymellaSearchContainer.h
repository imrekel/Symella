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
*  Name     : CSymellaContainer2 from SymellaContainer2.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
*  Description:
*     Declares container control for application.
* ============================================================================
*/

#ifndef SYMELLACONTAINER_H
#define SYMELLACONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <aknsfld.h> 
 
// FORWARD DECLARATIONS
class CEikTextListBox;
class CTextListBoxModel;

// CLASS DECLARATION

/**
 * CSymellaSearchContainer 
 *
 * Container window for the search view (it contains a list with
 * the current search results)
 */
class CSymellaSearchContainer : public CCoeControl
{
public: // Constructors and destructor
    
    /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
    void ConstructL(const TRect& aRect);

    /**
    * Destructor.
    */
    ~CSymellaSearchContainer();

	inline CEikTextListBox* ListBox();
	
	inline CAknSearchField* FindBox();
	
	void CreateFindBoxL();
	
	void DeleteFindBox();
	
	TInt CurrentItemIndex();


public:

	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
	
private:

	CAknSearchField* CreateFindBoxL( 
        CEikListBox* aListBox,
        CTextListBoxModel* aModel,
        CAknSearchField::TSearchFieldStyle aStyle );

private: // from CCoeControl

    void SizeChanged();
   
    TInt CountComponentControls() const;

  
    CCoeControl* ComponentControl(TInt aIndex) const;

private: //data
    
    CEikTextListBox* iListBox;
    
    CAknSearchField* iFindBox;
};


inline CEikTextListBox* CSymellaSearchContainer::ListBox() {
	return iListBox;
}

inline CAknSearchField* CSymellaSearchContainer::FindBox() {
	return iFindBox;
}

#endif

// End of File
