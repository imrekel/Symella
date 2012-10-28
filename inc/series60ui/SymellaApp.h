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
*  Name     : CSymellaApp from SymellaApp.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
*  Description:
*     Declares main application class.
* ============================================================================
*/

#ifndef SYMELLAAPP_H
#define SYMELLAAPP_H

// INCLUDES
#include <aknapp.h>

// CONSTANTS
// UID of the application
#ifdef EKA2
const TUid KUidSymella = { 0xA0001771 };
#else
const TUid KUidSymella = { 0x03EDB070 };
#endif

// CLASS DECLARATION

/**
* CSymellaApp application class.
* Provides factory to create concrete document object.
* 
*/
class CSymellaApp : public CAknApplication
    {
    
    public: // Functions from base classes
    private:

        /**
        * From CApaApplication, creates CSymellaDocument document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
        
        /**
        * From CApaApplication, returns application's UID (KUidSymella).
        * @return The value of KUidSymella.
        */
        TUid AppDllUid() const;
    };

#endif

// End of File

