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

#include "DummySettingItem.h"
#include <aknlists.h> 
#include <avkon.hrh>
#include <aknPopup.h>


CDummySettingItem::CDummySettingItem(TInt aIdentifier, TDes& aText) :
	CAknTextSettingItem(aIdentifier, aText)
{
}

void CDummySettingItem::LoadL() 
{ 
	CAknTextSettingItem::LoadL();
}

/*const TDesC& CDummySettingItem::SettingTextL( ) 
{
	return iText;
}*/

void CDummySettingItem::EditItemL(TBool /*aCalledFromMenu*/)
{
}

