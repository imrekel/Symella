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

#ifndef SAMESSAGECACHE_H
#define SAMESSAGECACHE_H

//INCLUDES
#include <e32base.h>
#include "SAPacket.h"

class TGuid;

/**
 * The messagecache, contains outgoing messageGuids, to match incoming messages to them
 */
class CSAMessageCache : public CBase
{
public:

	CSAMessageCache();

	~CSAMessageCache();

	void ConstructL();

	void AddGuidL(const TGuid& aGuid);

	TBool IsInCache(const TGuid& aGuid);

	void RemoveGuid(TGuid& aGuid);

	void MarkAsSearch(const TGuid& aGuid);

	inline TGuid& SearchGuid();

	inline TBool IsFirstSearch();

private:

	RArray<TGuid>			iMessageGuids;

	TGuid					iSearchGuid;

	TBool					iFirstSearch;
};


inline TGuid& CSAMessageCache::SearchGuid() {
	return iSearchGuid;
}

inline TBool CSAMessageCache::IsFirstSearch()
{
	return iFirstSearch;
}

#endif