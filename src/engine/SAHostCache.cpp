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

#include <in_sock.h>
#include "SAHostCache.h"
#include "SAPreferences.h"

const TInt KMaxHostCacheSize = 50;

const TInt KDefaultGoodQuality = 3;

void CSAHostCache::ConstructL()
{
	//load hostcaches
	PREFERENCES->ForceCreate();
	PREFERENCES->LoadHostCachesL(iUltrapeers,iLeafs,iUnidentifiedNodes,iWorkingNodes);

	//TEST: put some elements in the lists
	//TInetAddr ip(0x9842D7D4,4242);
	//AddUltrapeerL(ip);
	//TInetAddr ip3(0x9842D7D5,4141);
	//AddLeafL(ip3);
	//SetUltrapeerL(ip3);
	//save the hostcache
	//PREFERENCES->SaveHostCachesL(iUltrapeers,iLeafs,iUnidentifiedNodes);

	//ResetUltrapeers();
	//TInetAddr ip2;
	//GetUltrapeer(ip2);
	//TUint test2 = ip2.Port();
	//TInetAddr ip4;
	//GetUltrapeer(ip4);
	//TUint test = ip4.Port();
	////promote
	//PromoteUltrapeerL(ip4);
	//RemoveUltrapeer(ip2);
}


CSAHostCache::~CSAHostCache()
{
	iUltrapeers.ResetAndDestroy();
	iLeafs.ResetAndDestroy();
	iUnidentifiedNodes.ResetAndDestroy();
	iWorkingNodes.ResetAndDestroy();
}


void CSAHostCache::Reset()
{
	iUltrapeers.ResetAndDestroy();
	iLeafs.ResetAndDestroy();
	iUnidentifiedNodes.ResetAndDestroy();
	iWorkingNodes.ResetAndDestroy();
	
	iUltrapeersPos = 0;
	iWorkingsPos = 0;
	iUnidentifiedsPos = 0;
	iLeafsPos = 0;

	CTR->ConnectionObserver()->SetWorkingNodesCountL(0);
	CTR->ConnectionObserver()->SetLeafsCountL(0);
	CTR->ConnectionObserver()->SetUltrapeersCountL(0);
	CTR->ConnectionObserver()->SetUnidentifiedNodesCountL(0);
}


TBool CSAHostCache::IsUltrapeer(const TInetAddr& aAddress)
{
	for (TInt i=0;i<iUltrapeers.Count();i++)
	{
		if ((iUltrapeers[i]->iAddress == aAddress.Address()) &&
			(iUltrapeers[i]->iPort == aAddress.Port()))
		{
			return ETrue;
		}
	}
	return EFalse;
}

void CSAHostCache::SaveHostCacheL()
{
	PREFERENCES->SaveHostCachesL(iUltrapeers,iLeafs,iUnidentifiedNodes,iWorkingNodes);
}

//----------------- internal functions ------------------
TInt CSAHostCache::FindAddressPosInArray(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray)
{
	for (TInt i=0; i < aArray.Count(); i++)
		if ((aArray[i]->iAddress == aAddress.Address()) &&
			(aArray[i]->iPort == aAddress.Port()))
			return i;
	return KErrGeneral;
}

TInt CSAHostCache::RemoveFromArray(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray)
{
	TInt pos = FindAddressPosInArray(aAddress,aArray);
	if (pos != KErrGeneral)
	{
		delete aArray[pos];
		aArray.Remove(pos);
		ChangeArrayPos(aArray,pos,-1);
		return pos;
	}
	return KErrGeneral;
}

TInt CSAHostCache::AddToArrayL(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray)
{
	//look if it's in the array
	TInt pos = FindAddressPosInArray(aAddress,aArray);
	//if it's in the list, remove it, and insert it to the start of the list
	if (pos != KErrGeneral)
	{
		TSAHostCacheEntry* item = aArray[pos];
		CleanupStack::PushL(item);
		aArray.Remove(pos);
		aArray.Insert(item,0);
		CleanupStack::Pop();
		ChangeArrayPos(aArray,0,1);
		return pos;
	}
	else
	{
		TSAHostCacheEntry* entry = new (ELeave) TSAHostCacheEntry(aAddress);

		CleanupStack::PushL(entry);
		User::LeaveIfError(aArray.Insert(entry,0));
		CleanupStack::Pop();

		ChangeArrayPos(aArray,0,1);

		if (aArray.Count() > KMaxHostCacheSize)
		{
			delete aArray[aArray.Count()-1];
			aArray.Remove(aArray.Count()-1);
		}
		return 0;
	}
}

TInt CSAHostCache::MoveNodeL(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aTargetArray)
{
	TInt pos;
	pos = RemoveFromArray(aAddress,iUltrapeers);
	if (pos == KErrGeneral) pos = RemoveFromArray(aAddress,iLeafs);
	if (pos == KErrGeneral) pos = RemoveFromArray(aAddress,iWorkingNodes);
	if (pos == KErrGeneral) pos = RemoveFromArray(aAddress,iUnidentifiedNodes);
	//insert the address in the target array
	AddToArrayL(aAddress,aTargetArray);
	return KErrNone;
}

TInt CSAHostCache::ChangeArrayPos(RPointerArray<TSAHostCacheEntry>& aArray,TInt aGreaterThan,TInt aValue)
{
	if (&aArray == &iUltrapeers) 
		{if (iUltrapeersPos > aGreaterThan) {iUltrapeersPos += aValue;}}
	else if (&aArray == &iLeafs) 
		{if (iLeafsPos > aGreaterThan) {iLeafsPos += aValue;}}
	else if (&aArray == &iUnidentifiedNodes) 
		{if (iUnidentifiedsPos > aGreaterThan) {iUnidentifiedsPos += aValue;}}
	else if (&aArray == &iWorkingNodes) 
		{if (iWorkingsPos > aGreaterThan) {iWorkingsPos += aValue;}}
	return KErrNone;
}

TInt CSAHostCache::GetArrayPos(RPointerArray<TSAHostCacheEntry>& aArray)
{
	if (&aArray == &iUltrapeers) 
		{return iUltrapeersPos;}
	else if (&aArray == &iLeafs) 
		{return iLeafsPos;}
	else if (&aArray == &iUnidentifiedNodes) 
		{return iUnidentifiedsPos;}
	else if (&aArray == &iWorkingNodes) 
		{return iWorkingsPos;}
	return KErrGeneral;
}

TInt CSAHostCache::GetAddress(TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray)
{
	TInt arrayPos = GetArrayPos(aArray);
	if ((arrayPos != KErrGeneral)&&(arrayPos < aArray.Count()))
	{
		TSAHostCacheEntry* entry = aArray[arrayPos];
		aAddress.SetAddress(entry->iAddress);
		aAddress.SetPort(entry->iPort);
		ChangeArrayPos(aArray,-1,1);
		return KErrNone;
	}
	return KErrGeneral;
}

TInt CSAHostCache::PromoteAddressL(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray)
{
	//find the address in the array
	TInt pos = FindAddressPosInArray(aAddress,aArray);
	if (pos != KErrGeneral)
	{
		//remove it from the array, and insert it to the first position
		TSAHostCacheEntry* entry = aArray[pos];
		CleanupStack::PushL(entry);
		aArray.Remove(pos);
		aArray.Insert(entry,0);
		CleanupStack::Pop();
		ChangeArrayPos(aArray,0,1);
		return KErrNone;
	}
	return KErrGeneral;
}

TInt CSAHostCache::DegradeAddressL(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray)
{
	//find the address in the array
	TInt pos = FindAddressPosInArray(aAddress,aArray);
	if ((pos != KErrGeneral)&&(pos < aArray.Count() - 1))
	{
		//remove it from the array, and insert it to the last position
		TSAHostCacheEntry* entry = aArray[pos];
		CleanupStack::PushL(entry);
		aArray.Remove(pos);
		aArray.Append(entry);
		CleanupStack::Pop();
		ChangeArrayPos(aArray,pos,-1);
		return KErrNone;
	}
	return KErrGeneral;
}

void CSAHostCache::AddWorkingNodeL(const TInetAddr& aAddress)
{
	AddToArrayL(aAddress, iWorkingNodes); 
	CTR->ConnectionObserver()->SetWorkingNodesCountL(iWorkingNodes.Count());

	//find the address in the array
	TInt pos = FindAddressPosInArray(aAddress, iWorkingNodes);
	if ((pos != KErrGeneral)&&(pos < iWorkingNodes.Count() - 1))
	{
		iWorkingNodes[pos]->iQuality = KMaxQuality;
	}		
}

void CSAHostCache::PromoteWorkingNodeL(const TInetAddr& aAddress)
{
	if (PromoteAddressL(aAddress, iWorkingNodes) == KErrNone)
	{
		//find the address in the array
		TInt pos = FindAddressPosInArray(aAddress, iWorkingNodes);
		if ((pos != KErrGeneral)&&(pos < iWorkingNodes.Count() - 1))
		{
			iWorkingNodes[pos]->iQuality = KMaxQuality;
		}
	}
}


void CSAHostCache::DegradeWorkingNodeL(const TInetAddr& aAddress)
{
	if (DegradeAddressL(aAddress, iWorkingNodes) == KErrNone)
	{
		//find the address in the array
		TInt pos = FindAddressPosInArray(aAddress, iWorkingNodes);
		if ((pos != KErrGeneral)&&(pos < iWorkingNodes.Count() - 1))
		{
			if (iWorkingNodes[pos]->iQuality > 0) 
				iWorkingNodes[pos]->iQuality -= 1;
			else
			{
				delete iWorkingNodes[pos];
				iWorkingNodes.Remove(pos);
				ChangeArrayPos(iWorkingNodes, pos, -1);
			}
		}
	}

}

void CSAHostCache::SetWorkingNodeL(const TInetAddr& aAddress)
{
	MoveNodeL(aAddress,iWorkingNodes); 
	CTR->ConnectionObserver()->SetCountsL(
		iUltrapeers.Count(),iLeafs.Count(),
		iUnidentifiedNodes.Count(),
		iWorkingNodes.Count());

	//find the address in the array
	TInt pos = FindAddressPosInArray(aAddress, iWorkingNodes);
	if ((pos != KErrGeneral)&&(pos < iWorkingNodes.Count() - 1))
	{
		iWorkingNodes[pos]->iQuality = KMaxQuality;
	}
}