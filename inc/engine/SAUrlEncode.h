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

#ifndef SAURLENCODE_H
#define SAURLENCODE_H

#include <e32base.h>

/**
 * Contains functions for encoding urls (this feature is missing from
 * Symbian 6.0, from 7.0 you can use EscapeUtils::EscapeEncodeL).
 */
namespace NSAUrlEncode
{
	/**
	 * @param aDes the encodable url
	 * @return a buffer containing the encoded url (the caller takes
	 * ownership of the returned HBufC)
	 */
	HBufC8* EncodeUrlL(const TDesC8& aDes);

	/**
	 * Helper function
	 *
	 * @return true if aChar is an "unsafe" character (must be encoded)
	 */
	TBool IsUnsafe(TUint8 aChar);
}

#endif // SAURLENCODE_H