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

#ifndef SADOWNLOADMANAGER_H
#define SADOWNLOADMANAGER_H

// INCLUDES
#include <e32base.h>
#include <f32file.h>
#include "SADownload.h"

//FORWARD DECLARATIONS
class CSADownloadCandidate;
class TSAHash;

/**
 * The type of ordering used by the search results list.
 */
enum TSASearchResultsOrder
{
	EByFileName,
	EByArrival,
	EByAddressCount
};

/**
 * The status of a download.
 */
enum TSADownloadStatus
{
	EDownloadInProgress,
	EDownloadFailed,
	EDownloadComplete,
	EDownloadQueued
};

/**
 * The result of an attempt to start downloading a file.
 */
enum TSADownloadAttemptResult
{
	EFileExists,
	ESuccess,
	EAlreadyDownloading,
	EFailure,
	ECannotAccessDirectory,
	EInvalidPath,
	ECannotGetFreeSpace,
	ENotEnoughFreeSpace,
	ECannotOverwrite,
	ECannotCreateFile
};

/**
 */
class MSASearchResultsObserver
{
public:

	virtual void SearchStringChangedL(const TDesC8& aNewSearchString) = 0;
	virtual void InsertHitL(TInt aIndex, const TDesC8& aFileName, TUint32 aFileSize, TInt aHitCount, TBool aRefreshList = ETrue) = 0;
	virtual void ModifyHitCountL(TInt aIndexNum, TInt aHitCount) = 0;
	virtual void RemoveHitL(TInt aIndex) = 0;
	virtual void ResetHitsL() = 0;
};

class MSADownloadResultsObserver
{
public:

	virtual void InsertDownloadL(	TInt aIndex, 
									const TDesC8& aFileName, 
									TUint aFullSize, 
									TInt aPeerCount, 
									TSADownloadStatus aStatus) = 0;

	virtual void ModifyDownloadL(	TInt aIndex, 
									const TDesC8& aFileName, 
									TUint aFullSize, 
									TUint aDownloadedSize, 
									TInt aPeerCount, 					
									TSADownloadStatus aStatus) = 0;

	virtual void RemoveDownloadL(TInt aIndex) = 0;
};

/**
 * Class responsible for handling downloads and managing search results.
 */
class CSADownloadManager : public CBase
{
public:

	CSADownloadManager();

	~CSADownloadManager();

	void ConstructL();

	/**
	 * Processes a query hit and adds it to the search results ("download
	 * candidates") list
	 */
	void RegisterQueryHitL( const TDesC8& aFileName, 
							TUint32 aFileSize,
							TUint32 aFileIndex,
							TUint32 aAddress, 
							TUint aPort,
							const TSAHash& aHash);

	/**
	 * Returns a string buffer, containing detailed info about a particular query hit.
	 *
	 * The caller's responsible for deleting the returned buffer!
	 */
	HBufC* CreateHitInfoL(TInt aIndex);

	/** 
	 * Deletes the hits list.
	 */
	void ResetHitsList();

	void ChangeHitsOrderingL(TSASearchResultsOrder aOrder);

	/**
	 * Starts downloading a file.
	 *
	 * @param aIndex the index of the downloadable download candidate (query hit)
	 * @param aOverWrite in case the downloadable file already exists, if it's true then
	 * overwrites the file
	 */
	TSADownloadAttemptResult StartDownloadL(TInt aIndex, TBool aOverWrite = EFalse);

	/**
	 * Removes a download from the download list (if the download is in progress then closes
	 * the connections)
	 */
	void RemoveDownloadL(TInt aIndex, TBool aDeleteFile);

	/**
	 * Sets the search results observer (ui component)
	 */
	inline void SetSearchResultsObserver(MSASearchResultsObserver* aObserver);

	inline void SetDownloadObserver(MSADownloadResultsObserver* aObserver);

	void OnTimerL();

	void HandleDownloadCompletionL();

	void HandleDownloadFailureL(CSADownload* aDownload);

	void HandleDownloadSizeChangedL(CSADownload* aDownload);

	inline TInt MaxFragmentsPerDownload();

	inline RFs& FS();

	inline MSADownloadResultsObserver* DownloadObserver();
	
	inline MSASearchResultsObserver* SearchObserver();
	
	inline CSADownload* GetLastDownload();
	
	inline CSADownloadCandidate* DownloadCandidate(TInt aIndex);
	

	/**
	 * @return ETrue if the specified download has already started
	 */
	inline TBool IsDownloadStarted(TInt aIndex);

	/**
	 * @param aIndex the index of the download
	 * @return the full path for the specified download
	 */
	TPtrC GetDownloadFileName(TInt aIndex);

	HBufC* CreateDownloadStatusInfoL(TInt aPos);

	/**
	 * @param aIndex the index of the download
	 * @return the state of the specified download
	 */
	CSADownload::TSADownloadState GetDownloadState(TInt aIndex);

private:

	void RemoveHitWithLowestAddressCount();

private:

	TInt								iMaxSimultaneousDownloads;

	TInt								iMaxFragmentsPerDownload;

	TInt								iActiveDownloadCount;

	RFs									iFs;

	TInt								iMaxTotalHitCount;

	TInt								iTotalHitCount;

	TInt								iLowestHitCount;

	TSASearchResultsOrder				iSearchResultsOrder;

	MSASearchResultsObserver*			iSearchObserver;

	MSADownloadResultsObserver*			iDownloadObserver;

	/**
	 * The list of downloadable files (a file can have multiple
	 * download addresses)
	 */
	RPointerArray<CSADownloadCandidate>	iDlCandidates;

	RPointerArray<CSADownload>			iDownloads;
	
	TInt								iGreatestAddressCount;
};

inline CSADownload* CSADownloadManager::GetLastDownload()
{
	return iDownloads[iDownloads.Count()-1];
}

inline void CSADownloadManager::SetSearchResultsObserver(MSASearchResultsObserver* aObserver) {
	iSearchObserver = aObserver;
}

inline void CSADownloadManager::SetDownloadObserver(MSADownloadResultsObserver* aObserver)
{
	iDownloadObserver = aObserver;
}

inline TInt CSADownloadManager::MaxFragmentsPerDownload() {
	return iMaxFragmentsPerDownload;
}

inline RFs& CSADownloadManager::FS() {
	return iFs;
}

inline MSADownloadResultsObserver* CSADownloadManager::DownloadObserver()
{
	return iDownloadObserver;
}

inline TBool CSADownloadManager::IsDownloadStarted(TInt aIndex) {
	return iDownloads[aIndex]->IsStarted();
}

inline MSASearchResultsObserver* CSADownloadManager::SearchObserver() {
	return iSearchObserver;
}

inline CSADownloadCandidate* CSADownloadManager::DownloadCandidate(TInt aIndex) {
	return iDlCandidates[aIndex];
}

#endif
