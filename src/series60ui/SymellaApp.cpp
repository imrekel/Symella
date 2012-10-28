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
*  Name     : CSymellaApp from SymellaApp.cpp
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include    "SymellaApp.h"
#include    "SymellaDocument.h"

#ifdef EKA2
 #include <eikstart.h>
#endif

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymellaApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid CSymellaApp::AppDllUid() const
    {
    return KUidSymella;
    }

   
// ---------------------------------------------------------
// CSymellaApp::CreateDocumentL()
// Creates CSymellaDocument object
// ---------------------------------------------------------
//
CApaDocument* CSymellaApp::CreateDocumentL()
    {
    return CSymellaDocument::NewL( *this );
    }

// ================= OTHER EXPORTED FUNCTIONS ==============
//
// ---------------------------------------------------------
// NewApplication() 
// Constructs CSymellaApp
// Returns: created application object
// ---------------------------------------------------------
//
EXPORT_C CApaApplication* NewApplication()
    {
    return new CSymellaApp;
    }

#ifdef EKA2

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }

#else

// ---------------------------------------------------------
// E32Dll(TDllReason) 
// Entry point function for EPOC Apps
// Returns: KErrNone: No error
// ---------------------------------------------------------
//
GLDEF_C TInt E32Dll( TDllReason )
    {
    return KErrNone;
    }

#endif //#if defined(EKA2)

// End of File
