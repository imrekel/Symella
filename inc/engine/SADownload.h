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

#ifndef SADOWNLOAD_H
#define SADOWNLOAD_H

// INCLUDES
#include <e32base.h>
#include "SAHash.h"
#include <f32file.h>

// FORWARD DECLARATIONS
class CSADownloadCandidate;
class CSADownloadAddress;
class CSAFragment;

enum TSAFragmentDeleted
{
	EFragmentDeleted,
	EFragmentNotDeleted
};



/**
 */
class CSADownload : public CBase
{
public:

	enum TSADownloadState
	{
		EDownloading,
		EComplete,
		EWaiting,
		EFailure
	};
	
	CSADownload();

	~CSADownload();

	void ConstructL(const CSADownloadCandidate& aDlCandidate, 
					const RFs& session, 
					const TDesC& aDownloadPath);

	void AddAddressL(TUint32 aAddress, 
					 TUint aPort, 
					 const TDesC8& aFileName, 
					 TUint32 aFileIndex);

	void OnTimerL();

	TInt StartDownloadingL(RFs& aFs);

public: // accessors

	inline TPtrC8 Name() const;

	inline TPtrC FullPath() const;

	inline TUint32 Size() const;

//	inline TInt AddressCount() const;

	inline const TSAHash& Hash() const;	

	inline TSADownloadState State() const;

	inline TInt DownloadingFragmentCount() const;	

//	void HandleFragmentCompletionL(CSAFragment* aFragment);

//	TSAFragmentDeleted HandleFragmentFailureL(CSAFragment* aFragment);

	void IncreaseDownloadedSizeL(TUint aSize);

	void DeleteFragments();

	void DeleteAddresses();

	void SetFailedL();

	inline TUint DownloadedSize();

	inline TInt AvailableAddressCount();

	inline TInt UsedAddressCount();

	inline TInt FragmentCount();

	inline void RemoveDlCandidate();

	inline CSADownloadCandidate* DlCandidate();

	/**
	 * @return ETrue if the download has already started in the past (doesn't mean that the download
	 * is currently in progress)
	 */
	inline TBool IsStarted() const;

private:

	void MoveAddressToAvailableFrontL(CSADownloadAddress* aAddress);
	void MoveAddressToAvailableBackL(CSADownloadAddress* aAddress);

private:

	TBool								iDownloadStarted;

	TInt								iDownloadingFragmentCount;

	TSADownloadState					iState;

	TSAHash								iHash;

	HBufC8*								iName;

	HBufC*								iPath;

	TUint32								iSize;

	RPointerArray<CSADownloadAddress>	iAvailableAddresses;

	RPointerArray<CSADownloadAddress>	iUsedAddresses;

	RPointerArray<CSAFragment>			iFragments;

	TUint								iDownloadedSize;

	TUint								iFinishedFragmentsSize;

	CSADownloadCandidate*				iDlCandidate;
};

inline CSADownload::TSADownloadState CSADownload::State() const {
	return iState;
}

inline TPtrC8 CSADownload::Name() const {
	return *iName;
}

const TSAHash& CSADownload::Hash() const {
	return iHash;
}

inline TUint32 CSADownload::Size() const {
	return iSize;
}

inline TUint CSADownload::DownloadedSize()
{
	return iDownloadedSize;
}

/*inline TInt CSADownload::AddressCount() const {
	return iAddresses.Count();
}*/

inline TInt CSADownload::DownloadingFragmentCount() const {
	return iDownloadingFragmentCount;
}

inline TInt CSADownload::AvailableAddressCount() {
	return iAvailableAddresses.Count();
}

inline TInt CSADownload::UsedAddressCount() {
	return iUsedAddresses.Count();
}

inline TInt CSADownload::FragmentCount() {
	return iFragments.Count();
}

inline void CSADownload::RemoveDlCandidate() {
	iDlCandidate = 0;
}

inline CSADownloadCandidate* CSADownload::DlCandidate() {
	return iDlCandidate;
}

inline TPtrC CSADownload::FullPath() const {
	return TPtrC(*iPath);
}

inline TBool CSADownload::IsStarted() const {
	return iDownloadStarted;
}

#endif
