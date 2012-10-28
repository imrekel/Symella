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

#include "SAUrlEncode.h"

_LIT8(KUnsafeChars, "\"<>%\\^[]`+$,@:;/!#?=&()");

HBufC8* NSAUrlEncode::EncodeUrlL(const TDesC8& aDes)
{
	TInt unsafeCount = 0;

	//count the number of encodable characters
	{
		for (TInt i=0; i<aDes.Length(); i++)
			if (IsUnsafe(aDes[i]))
				unsafeCount++;
	}

	HBufC8* encoded = HBufC8::NewL(aDes.Length() + unsafeCount*2);
	TPtr8 ptr(encoded->Des());

	{
		for (TInt i=0; i<aDes.Length(); i++)
		{
			if (IsUnsafe(aDes[i]))
			{
				TBuf8<3> buf;
				buf.Format(_L8("%%%X"), aDes[i]);
				ptr.Append(buf);
			}
			else
				ptr.Append(&aDes[i], 1);

		}
	}

	return encoded;
}


TBool NSAUrlEncode::IsUnsafe(TUint8 aChar)
{
	if (KUnsafeChars().Find(&aChar, 1) != KErrNotFound)
		return ETrue;

	 if ((aChar > 32) && (aChar < 123))
		 return EFalse;
	 else
		 return ETrue;
}