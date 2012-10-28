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

#ifndef CSAFRAGMENT_H
#define CSAFRAGMENT_H

#include <f32file.h>

class CSADownloadAddress;
class CSAFragmentDownloader;
class CSADownload;

/**
 * Class representing a fragment of a downloadable file.
 */
class CSAFragment : public CBase
{
public: // enums

	enum TSAFragmentState
	{
		EFinished,
		EDownloading,
		EWaiting,
		EFailed
	};

	enum TSAFailureReason
	{
		ECannotConnect,
		EDisconnectedWhileDownloading,
		EHandshakeFailed,
		EHandshakeFailedServiceUnavailable,
		EServentFull,
		EGeneral
	};

public:

	CSAFragment(TUint aStartOffset, TUint aSize, CSADownload& aParentDownload);

	~CSAFragment();

	TInt StartDownloadingL(	RFs& aFs, const 
							CSADownloadAddress& aAddress, 
							const TDesC& aTarget);

	void WriteToFileL(const TDesC8& aBuf);

	void OnTimerL();

	void SetFailed(TSAFailureReason aReason);

	void SetWaiting();

public: // accessors

	inline TSAFragmentState State() const ;
	
	inline TUint DownloadedSize() const;

	inline TUint StartOffset() const;

	inline TUint Size() const;

	inline CSADownloadAddress* Address() const;

	inline TUint LastDataSize() const;

	inline CSADownload& ParentDownload() const;

	inline TSAFailureReason FailureReason() const;

	TBool IsDownloaderActive();

private:

	CSADownloadAddress* iAddress;

	CSAFragmentDownloader* iDownloader;

	CSADownload& iParentDownload;

	RFile iFile;

	TUint iStartOffset;

	TUint iSize;

	TUint iDownloadedSize;

	TSAFragmentState iState;

	TSAFailureReason iFailureReason;

	TUint iLastDataSize;
};

inline CSADownload& CSAFragment::ParentDownload() const {
	return iParentDownload;
}

inline CSAFragment::TSAFragmentState CSAFragment::State() const {
	return iState;
}

inline TUint CSAFragment::DownloadedSize() const {
	return iDownloadedSize;
}

inline TUint CSAFragment::StartOffset() const {
	return iStartOffset;
}

inline TUint CSAFragment::Size() const {
	return iSize;
}

inline CSADownloadAddress* CSAFragment::Address() const {
	return iAddress;
}

inline TUint CSAFragment::LastDataSize() const {
	return iLastDataSize;
}

inline CSAFragment::TSAFailureReason CSAFragment::FailureReason() const {
	return iFailureReason;
}

#endif