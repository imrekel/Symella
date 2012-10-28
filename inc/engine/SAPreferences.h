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

#ifndef __SAPREFERENCES_H__
#define __SAPREFERENCES_H__

#include <coemain.h>
#include <e32base.h>
#include <in_sock.h>
#include "SAWebCacheListEntry.h"

//Singleton uid
const TUid KUidSymellaPreferences = { 0x10000EF3 };

#define PREFERENCES CSAPreferences::InstanceL()

class TSAHostCacheEntry;
class TInetAddr;
class RFile;
class CDesC8Array;
class CSAWebCacheListEntry;


/**
 * MSAWebCacheListObserver
 */
class MSAWebCacheListObserver
{
public:

	virtual void WebCacheListChangedL() = 0;
};

/**
 * CSAPreferences
 */
class CSAPreferences : public CCoeStatic
{
public:

	static CSAPreferences* InstanceL();

	~CSAPreferences();

	void ConstructL();
	
	inline void ForceCreate();

	TInt LoadSettingsL();
	
	TInt SaveSettingsL();
	
	TInt SaveSettingL(RFile& aFile, const TDesC8& aName, const TDesC8& aValue);
	
	TInt LoadSettingL(TDesC8& aRow, TDes8& aName, TDes8& aValue);

	TInt LoadHostCachesL(RPointerArray<TSAHostCacheEntry>& aUltrapeers,
						 RPointerArray<TSAHostCacheEntry>& aLeafs,
						 RPointerArray<TSAHostCacheEntry>& aUnidentifieds,
						 RPointerArray<TSAHostCacheEntry>& aWorkingNodes);
						
	TInt SaveHostCachesL(RPointerArray<TSAHostCacheEntry>& aUltrapeers,
						 RPointerArray<TSAHostCacheEntry>& aLeafs,
						 RPointerArray<TSAHostCacheEntry>& aUnidentifieds,
						 RPointerArray<TSAHostCacheEntry>& aWorkingNodes);
						 
public: // webcache	
					
	TInt LoadWebCachesL();
	
	TInt SaveWebCachesL();
	
	TInt GetNextWebCacheL(TDes8& aUrl);
	
	const TDesC8& GetLastWebCacheL();
	
	void AddWebCacheL(const TDesC8& aUrl, TInt aIndex = 0);
	
	void RemoveWebCacheL(TInt aIndex);
	
	TInt SetWebCacheUrlsL(const TDesC& aUrls);
	
	HBufC* GetWebCacheUrls();
	
	inline void SetWebCacheListObserver(MSAWebCacheListObserver* aObserver);
	
	inline void RemoveWebCacheListObserver();
	
	void MoveWebCacheToTopL(const TDesC8& aWebcache);
	
	inline TInt LastWebcacheIndex() const;
	
public:	//getter

	inline TInt		GetPort();
	
	inline TPtrC	GetDestDir();
	
	inline TInt		GetMaxStoredHits();
	
	inline TInt		GetLowestNonDeletableHitCount();
	
	inline TBool	GetUseGWC();
	
	inline TInt		GetUserId();
	
	inline TBool	GetEnableProfiling();
	
	inline TBool	GetEnableConnectionProfiling();
	
	inline TInt WebCacheCount() const;
	
	inline CSAWebCacheListEntry* WebCache(TInt aIndex);
	
	/**
	 * @return the name of the currently selected internet access point or KNullDesC if
	 * it's unset
	 */
	inline const TDesC& AccessPointName() const;
		
	inline TUint32 AccessPointId() const;

	//setter
	inline void SetPort(const TInt aPort);	
	inline void SetMaxStoredHits(const TInt aMaxStoredHits);	
	inline void SetLowestNonDeletableHitCount(const TInt aLowestNonDeletableHitCount);
	inline void SetUseGWC(const TBool aUseGWC);
	inline void SetUserId(const TInt aUserId);
	inline void SetEnableProfiling(const TBool aEnable);
	inline void SetEnableConnectionProfiling(const TBool aEnable);
	void SetDestDir(const TDesC& aDestDir);
	void SetAccessPointL(const TDesC& aAccessPointName, TUint32 aAccessPointId);

protected:

	CSAPreferences() : CCoeStatic(KUidSymellaPreferences) {}

private:

	void CreateDefaultSettingsFileL();
	
	void CreateDefaultHostCacheFileL();
	
	void CreateDefaultGWebCacheFileL();

	TInt	iPort;
	HBufC*	iDestDir;
	TInt	iUserId;	
	TInt	iMaxStoredHits;
	TInt	iLowestNonDeletableHitCount;
	TBool	iUseGWC;
	TBool	iUseSwarming;
	TBool	iEnableProfiling;
	TBool	iEnableConnectionProfiling;
	
	HBufC8*  iLastWebCache;
	
	TInt	iLastWebcacheIndex;
	
	TInt	iGWCPos;

	RFs		iFsSession;
	
	HBufC* iAccessPointName;
	
	TUint32 iAccessPointId;
	
	RPointerArray<CSAWebCacheListEntry> iWebCaches;
	
	MSAWebCacheListObserver* iWebCacheListObserver;

};

inline TBool CSAPreferences::GetUseGWC()
{
	return iUseGWC;
}

inline TBool CSAPreferences::GetEnableProfiling()
{
	return iEnableProfiling;
}

inline TBool CSAPreferences::GetEnableConnectionProfiling()
{
	return iEnableConnectionProfiling;
}


inline void CSAPreferences::ForceCreate()
{
}

inline TInt CSAPreferences::GetPort()
{
	return iPort;
}

inline TInt CSAPreferences::GetUserId()
{
	return iUserId;
}

inline TPtrC CSAPreferences::GetDestDir()
{
	return *iDestDir;
}

inline void CSAPreferences::SetPort(const TInt aPort)
{
	iPort = aPort;
	SaveSettingsL();
}

inline TInt CSAPreferences::GetMaxStoredHits()
{
	return iMaxStoredHits;
}

inline TInt CSAPreferences::GetLowestNonDeletableHitCount()
{
	return iLowestNonDeletableHitCount;
}

inline void CSAPreferences::SetMaxStoredHits(const TInt aMaxStoredHits)
{
	iMaxStoredHits = aMaxStoredHits;
	SaveSettingsL();
}

inline void CSAPreferences::SetUserId(const TInt aUserId)
{
	iUserId = aUserId;
	SaveSettingsL();
}

inline void CSAPreferences::SetLowestNonDeletableHitCount(const TInt aLowestNonDeletableHitCount)
{
	iLowestNonDeletableHitCount = aLowestNonDeletableHitCount;
	SaveSettingsL();
}

inline void CSAPreferences::SetUseGWC(const TBool aUseGWC)
{
	iUseGWC = aUseGWC;
	SaveSettingsL();
}

inline void CSAPreferences::SetEnableProfiling(const TBool aEnable)
{
	iEnableProfiling = aEnable;
	SaveSettingsL();
}

inline void CSAPreferences::SetEnableConnectionProfiling(const TBool aEnable)
{
	iEnableConnectionProfiling = aEnable;
	SaveSettingsL();
}

inline const TDesC& CSAPreferences::AccessPointName() const {
	if (iAccessPointName)
		return *iAccessPointName;
	return KNullDesC;
}
	
inline TUint32 CSAPreferences::AccessPointId() const {
	return iAccessPointId;
}

inline TInt CSAPreferences::WebCacheCount() const {
	return iWebCaches.Count();	
}
	
inline CSAWebCacheListEntry* CSAPreferences::WebCache(TInt aIndex) {
	return iWebCaches[aIndex];
}

inline void CSAPreferences::SetWebCacheListObserver(MSAWebCacheListObserver* aObserver) {
	iWebCacheListObserver = aObserver;
}
	
inline void CSAPreferences::RemoveWebCacheListObserver() {
	iWebCacheListObserver = NULL;
}

inline TInt CSAPreferences::LastWebcacheIndex() const {
	return iLastWebcacheIndex + 1;
}

#endif
