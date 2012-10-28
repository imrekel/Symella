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

#ifndef SAHOSTCACHE_H
#define SAHOSTCACHE_H

//INCLUDES
#include <e32base.h>
#include "SAControl.h"

//FORWARD DECLARATIONS
class TInetAddr;

/**
 * An entry in the host cache, contains information about
 * a gnutella node.
 */
class TSAHostCacheEntry
{
public:

	TSAHostCacheEntry(const TInetAddr& aAddress);

	TSAHostCacheEntry(const TInetAddr& aAddress, TInt aQuality);

public:

	TUint32		iAddress;
	TUint		iPort;
	TInt		iQuality;
};

const TInt KMaxQuality = 5;


/**
 * The host cache
 */
class CSAHostCache : public CBase
{
public:

	inline void AddUltrapeerL(const TInetAddr& aAddress);
	void AddWorkingNodeL(const TInetAddr& aAddress);
	inline void AddLeafL(const TInetAddr& aAddress);
	inline void AddUnidentifiedL(const TInetAddr& aAddress);

	inline TInt UltrapeerCount();
	inline TInt LeafsCount();
	inline TInt UnidentifiedCount();
	inline TInt WorkingNodesCount();

	inline TInt GetUltrapeer(TInetAddr& aAddress);
	inline TInt GetLeaf(TInetAddr& aAddress);
	inline TInt GetUnidentified(TInetAddr& aAddress);
	inline TInt GetWorkingNode(TInetAddr& aAddress);

	inline void SetUltrapeerL(const TInetAddr& aAddress);
	inline void SetLeafL(const TInetAddr& aAddress);
	inline void SetUnidentifiedL(const TInetAddr& aAddress);
	void SetWorkingNodeL(const TInetAddr& aAddress);

	inline TInt RemoveUltrapeer(const TInetAddr& aAddress);
	inline TInt RemoveLeaf(const TInetAddr& aAddress);
	inline TInt RemoveUnidentified(const TInetAddr& aAddress);
	inline TInt RemoveWorkingNode(const TInetAddr& aAddress);
	inline TInt RemoveNode(const TInetAddr& aAddress);

	inline void PromoteUltrapeerL(const TInetAddr& aAddress);
	inline void DegradeUltrapeerL(const TInetAddr& aAddress);
	void PromoteWorkingNodeL(const TInetAddr& aAddress);
	void DegradeWorkingNodeL(const TInetAddr& aAddress);

	inline void ResetUltrapeers();
	inline void ResetLeafs();
	inline void ResetUnidentifieds();
	inline void ResetWorkingNodes();

	inline void ShowCountsL();

	void SaveHostCacheL();

	/**
	 * Deletes all stored nodes from the cache
	 */
	void Reset();

	inline const TSAHostCacheEntry& Ultrapeer(TInt aIndex);

	TBool IsUltrapeer(const TInetAddr& aAddress);

	~CSAHostCache();

	void ConstructL();

private:
	//internally used functions
	TInt FindAddressPosInArray(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray);
	TInt RemoveFromArray(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray);
	TInt AddToArrayL(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray);
	TInt MoveNodeL(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aTargetArray);
	TInt ChangeArrayPos(RPointerArray<TSAHostCacheEntry>& aArray,TInt aGreaterThan,TInt aValue);
	TInt GetArrayPos(RPointerArray<TSAHostCacheEntry>& aArray);
	TInt GetAddress(TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray);
	TInt PromoteAddressL(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray);
	TInt DegradeAddressL(const TInetAddr& aAddress,RPointerArray<TSAHostCacheEntry>& aArray);

private:

	RPointerArray<TSAHostCacheEntry>	iUltrapeers;
	RPointerArray<TSAHostCacheEntry>	iLeafs;
	RPointerArray<TSAHostCacheEntry>	iUnidentifiedNodes;
	RPointerArray<TSAHostCacheEntry>	iWorkingNodes;
	TInt						iUltrapeersPos;
	TInt						iLeafsPos;
	TInt						iUnidentifiedsPos;
	TInt						iWorkingsPos;
};

inline const TSAHostCacheEntry& CSAHostCache::Ultrapeer(TInt aIndex)	{return *iUltrapeers[aIndex];}

//--------------- Count functions -------------
inline TInt CSAHostCache::UltrapeerCount()								{return iUltrapeers.Count();}
inline TInt CSAHostCache::UnidentifiedCount()							{return iUnidentifiedNodes.Count();}
inline TInt CSAHostCache::WorkingNodesCount()							{return iWorkingNodes.Count();}
inline TInt CSAHostCache::LeafsCount()									{return iLeafs.Count();}

//-------------- reset counter functions -----------
inline void CSAHostCache::ResetUltrapeers()								{iUltrapeersPos=0;}
inline void CSAHostCache::ResetLeafs()									{iLeafsPos=0;}
inline void CSAHostCache::ResetUnidentifieds()							{iUnidentifiedsPos=0;}
inline void CSAHostCache::ResetWorkingNodes()							{iWorkingsPos=0;}

//------------------- Add functions -----------------------
inline void CSAHostCache::AddUltrapeerL(const TInetAddr& aAddress)		{AddToArrayL(aAddress,iUltrapeers); CTR->ConnectionObserver()->SetUltrapeersCountL(iUltrapeers.Count());}
//inline void CSAHostCache::AddWorkingNodeL(const TInetAddr& aAddress)	{AddToArrayL(aAddress,iWorkingNodes); CTR->ConnectionObserver()->SetWorkingNodesCountL(iWorkingNodes.Count());}
inline void CSAHostCache::AddLeafL(const TInetAddr& aAddress)			{AddToArrayL(aAddress,iLeafs); CTR->ConnectionObserver()->SetLeafsCountL(iLeafs.Count());}
inline void CSAHostCache::AddUnidentifiedL(const TInetAddr& aAddress)	{AddToArrayL(aAddress,iUnidentifiedNodes); CTR->ConnectionObserver()->SetUnidentifiedNodesCountL(iUnidentifiedNodes.Count());}

//------------------ Get functions ----------------
inline TInt CSAHostCache::GetUltrapeer(TInetAddr& aAddress)				{return GetAddress(aAddress,iUltrapeers);}
inline TInt CSAHostCache::GetLeaf(TInetAddr& aAddress)					{return GetAddress(aAddress,iLeafs);}
inline TInt CSAHostCache::GetUnidentified(TInetAddr& aAddress)			{return GetAddress(aAddress,iUnidentifiedNodes);}
inline TInt CSAHostCache::GetWorkingNode(TInetAddr& aAddress)			{return GetAddress(aAddress,iWorkingNodes);}

//------------------ set address functions -------------
inline void CSAHostCache::SetUltrapeerL(const TInetAddr& aAddress)		{MoveNodeL(aAddress,iUltrapeers); CTR->ConnectionObserver()->SetCountsL(iUltrapeers.Count(),iLeafs.Count(),iUnidentifiedNodes.Count(),iWorkingNodes.Count());}
inline void CSAHostCache::SetLeafL(const TInetAddr& aAddress)			{MoveNodeL(aAddress,iLeafs); CTR->ConnectionObserver()->SetCountsL(iUltrapeers.Count(),iLeafs.Count(),iUnidentifiedNodes.Count(),iWorkingNodes.Count());}
inline void CSAHostCache::SetUnidentifiedL(const TInetAddr& aAddress)	{MoveNodeL(aAddress,iUnidentifiedNodes); CTR->ConnectionObserver()->SetCountsL(iUltrapeers.Count(),iLeafs.Count(),iUnidentifiedNodes.Count(),iWorkingNodes.Count());}
//inline void CSAHostCache::SetWorkingNodeL(const TInetAddr& aAddress)	{MoveNodeL(aAddress,iWorkingNodes); CTR->ConnectionObserver()->SetCountsL(iUltrapeers.Count(),iLeafs.Count(),iUnidentifiedNodes.Count(),iWorkingNodes.Count());}

//----------------- remove node functions --------------
inline TInt CSAHostCache::RemoveUltrapeer(const TInetAddr& aAddress)	
{
	TInt result = RemoveFromArray(aAddress,iUltrapeers); 
	CTR->ConnectionObserver()->SetUltrapeersCountL(iUltrapeers.Count());
	return result;
}
inline TInt CSAHostCache::RemoveLeaf(const TInetAddr& aAddress)			
{
	TInt result = RemoveFromArray(aAddress,iLeafs); 
	CTR->ConnectionObserver()->SetLeafsCountL(iLeafs.Count());
	return result;
}
inline TInt CSAHostCache::RemoveUnidentified(const TInetAddr& aAddress)	
{
	TInt result = RemoveFromArray(aAddress,iUnidentifiedNodes); 
	CTR->ConnectionObserver()->SetUnidentifiedNodesCountL(iUnidentifiedNodes.Count());
	return result;
}
inline TInt CSAHostCache::RemoveWorkingNode(const TInetAddr& aAddress)	
{
	TInt result = RemoveFromArray(aAddress,iWorkingNodes); 
	CTR->ConnectionObserver()->SetWorkingNodesCountL(iWorkingNodes.Count());
	return result;
}
inline TInt CSAHostCache::RemoveNode(const TInetAddr& aAddress)
{
	TInt res = RemoveFromArray(aAddress,iUltrapeers);
	if (res == KErrGeneral) res = RemoveFromArray(aAddress,iLeafs);
	if (res == KErrGeneral) res = RemoveFromArray(aAddress,iWorkingNodes);
	if (res == KErrGeneral) res = RemoveFromArray(aAddress,iUnidentifiedNodes);
	CTR->ConnectionObserver()->SetCountsL(iUltrapeers.Count(),iLeafs.Count(),iUnidentifiedNodes.Count(),iWorkingNodes.Count());
	return KErrNone;
}

//---------------------- promote and degrade functions --------
inline void CSAHostCache::PromoteUltrapeerL(const TInetAddr& aAddress)	{PromoteAddressL(aAddress,iUltrapeers);}
inline void CSAHostCache::DegradeUltrapeerL(const TInetAddr& aAddress)	{DegradeAddressL(aAddress,iUltrapeers);}
//inline void CSAHostCache::PromoteWorkingNodeL(const TInetAddr& aAddress) {PromoteAddressL(aAddress,iWorkingNodes);}
//inline void CSAHostCache::DegradeWorkingNodeL(const TInetAddr& aAddress) {DegradeAddressL(aAddress,iWorkingNodes);}

inline void CSAHostCache::ShowCountsL()
{
	CTR->ConnectionObserver()->SetCountsL(iUltrapeers.Count(),iLeafs.Count(),iUnidentifiedNodes.Count(),iWorkingNodes.Count());
}

#endif
