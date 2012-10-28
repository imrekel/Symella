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

#include "SAMessageCache.h"
#include "SAPacket.h"

const TInt KMaxMessageCacheSize = 50;

void CSAMessageCache::ConstructL()
{
}

void CSAMessageCache::AddGuidL(const TGuid& aGuid)
{
	//if there was none in the list, add it to the cache
	if (IsInCache(aGuid))
	{
		iMessageGuids.Insert(aGuid, 0);
//		if (iSearchGuid > -1) iSearchGuid++;
		if (iMessageGuids.Count() > KMaxMessageCacheSize)
		{
			iMessageGuids.Remove(0);
		}
	}
}

TBool CSAMessageCache::IsInCache(const TGuid& aGuid)
{
	for (TInt i=0; i<iMessageGuids.Count(); i++)
		if (iMessageGuids[i] == aGuid)
			return ETrue;
	return EFalse;
}

void CSAMessageCache::RemoveGuid(TGuid& aGuid)
{
	for (TInt i=0;i<iMessageGuids.Count();i++)
		if (iMessageGuids[i] == aGuid)
		{			
			iMessageGuids.Remove(i);
			break;
		}
}

CSAMessageCache::~CSAMessageCache()
{
	iMessageGuids.Close();
}

CSAMessageCache::CSAMessageCache()
{
	iFirstSearch = ETrue;
}

void CSAMessageCache::MarkAsSearch(const TGuid& aGuid)
{
	RemoveGuid(iSearchGuid);
	
	iSearchGuid = aGuid;

	iFirstSearch = EFalse;

/*	if ((iSearchGuid > -1) && (iSearchGuid < iMessageGuids.Count()))
	{
		//remove the guid at the iSearchGuid position
		delete iMessageGuids[iSearchGuid];
		iMessageGuids.Remove(iSearchGuid);
	}
	iSearchGuid = -1;

	for (TInt i=0;i<iMessageGuids.Count();i++)
		if (iMessageGuids[i] == aGuid)
		{
			iSearchGuid = i;
			break;
		}*/
}