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

#ifndef SAHASH_H
#define SAHASH_H

//INCLUDES
#include <e32base.h>

/**
 * A hash value
 */
class TSAHash
{
public:

	inline TSAHash();

	inline TSAHash(const TSAHash& aHash);

	TSAHash(const TDesC8& aBuf);

	TBool IsValid() const;

	inline const TSAHash& operator=(const TSAHash& aHash);

	inline TBool operator==(const TSAHash& aHash) const;

	inline void Hash(TDes8& aHash);

private:

	TBuf8<32> iHash;
};


inline const TSAHash& TSAHash::operator=(const TSAHash& aHash) {
	iHash = aHash.iHash;

	return *this;
}

inline TBool TSAHash::operator==(const TSAHash& aHash) const {
	return (iHash == aHash.iHash);
}

inline TSAHash::TSAHash() {
	iHash.SetLength(32);
	iHash.FillZ();
}

inline TSAHash::TSAHash(const TSAHash& aHash) {
	*this = aHash;
}

inline void TSAHash::Hash(TDes8& aHash)
{
	aHash.Copy(iHash);
}

#endif
