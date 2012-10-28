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
*  Name     : CSymellaDocument from SymellaDocument.h
*  Part of  : Symella
*  Created  : 3/12/2005 by Balázs Molnár, Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include "SymellaDocument.h"
#include "SymellaAppUi.h"

// ================= MEMBER FUNCTIONS =======================

// constructor
CSymellaDocument::CSymellaDocument(CEikApplication& aApp)
: CAknDocument(aApp)    
    {
    }

// destructor
CSymellaDocument::~CSymellaDocument()
    {
    }

// EPOC default constructor can leave.
void CSymellaDocument::ConstructL()
    {
    }

// Two-phased constructor.
CSymellaDocument* CSymellaDocument::NewL(
        CEikApplication& aApp)     // CSymellaApp reference
    {
    CSymellaDocument* self = new (ELeave) CSymellaDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }
    
// ----------------------------------------------------
// CSymellaDocument::CreateAppUiL()
// constructs CSymellaAppUi
// ----------------------------------------------------
//
CEikAppUi* CSymellaDocument::CreateAppUiL()
    {
    return new (ELeave) CSymellaAppUi;
    }

// End of File  
