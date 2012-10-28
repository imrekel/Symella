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

#ifndef SADOWNLOADCANDIDATE_H
#define SADOWNLOADCANDIDATE_H

// INCLUDES
#include <e32base.h>
#include "SAHash.h"

// FORWARD DECLARATIONS
class CSADownloadAddress;
class CSADownload;

/**
 */
class CSADownloadCandidate : public CBase
{
public:

	inline CSADownloadCandidate(TUint32 aSize);

	inline CSADownloadCandidate(TUint32 aSize, const TSAHash& aHash);

	~CSADownloadCandidate();

	void ConstructL(const TDesC8& aName);

	void AddAddressL(TUint32 aAddress, 
					 TUint aPort, 
					 const TDesC8& aFileName, 
					 TUint32 aFileIndex);

	inline CSADownloadAddress* Address(TInt aIndex);

	inline CSADownloadAddress* Address(TInt aIndex) const;

	inline TPtrC8 Name() const;

	inline TUint32 Size() const;

	inline TInt AddressCount() const;

	inline const TSAHash& Hash() const;

	inline void SetDownload(CSADownload* aDownload);

	inline CSADownload* GetDownload();

	static TInt CompareAddressCount(
		const CSADownloadCandidate& aFirst, 
		const CSADownloadCandidate& aSecond);

	static TInt CompareFileName(
		const CSADownloadCandidate& aFirst, 
		const CSADownloadCandidate& aSecond);

private:

	TSAHash		iHash;

	HBufC8*		iName;

	TUint32		iSize;

	RPointerArray<CSADownloadAddress> iAddresses;

	CSADownload*	iDownload;
};

inline CSADownloadCandidate::CSADownloadCandidate(TUint32 aSize) : iSize(aSize)
{}

inline CSADownloadCandidate::CSADownloadCandidate(TUint32 aSize, const TSAHash& aHash) : iSize(aSize), iHash(aHash)
{}

inline TPtrC8 CSADownloadCandidate::Name() const {
	return *iName;
}

const TSAHash& CSADownloadCandidate::Hash() const {
	return iHash;
}

inline TUint32 CSADownloadCandidate::Size() const {
	return iSize;
}

inline TInt CSADownloadCandidate::AddressCount() const {
	return iAddresses.Count();
}

inline CSADownloadAddress* CSADownloadCandidate::Address(TInt aIndex) {
	return iAddresses[aIndex];
}

inline CSADownloadAddress* CSADownloadCandidate::Address(TInt aIndex) const {
	return const_cast<CSADownloadAddress*>(iAddresses[aIndex]);
}

inline void CSADownloadCandidate::SetDownload(CSADownload* aDownload) {
	iDownload = aDownload;
}

inline CSADownload* CSADownloadCandidate::GetDownload() {
	return iDownload;
}

#endif
