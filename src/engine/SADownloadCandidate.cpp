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

#include "SADownloadCandidate.h"
#include "SADownloadAddress.h"
#include "SADownload.h"


void CSADownloadCandidate::ConstructL(const TDesC8& aName)
{
	iName = aName.AllocL();
}


CSADownloadCandidate::~CSADownloadCandidate()
{
	if (iDownload)
		iDownload->RemoveDlCandidate();

	delete iName;
	iAddresses.ResetAndDestroy();
}


void CSADownloadCandidate::AddAddressL(	TUint32 aAddress, 
										TUint aPort, 
										const TDesC8& aFileName, 
										TUint32 aFileIndex)
{
	CSADownloadAddress* address = new (ELeave) CSADownloadAddress;
	CleanupStack::PushL(address);
	address->ConstructL(aAddress, aPort, aFileName, aFileIndex);
	User::LeaveIfError(iAddresses.Append(address));
	CleanupStack::Pop();

	if (iDownload)
		iDownload->AddAddressL(aAddress, aPort, aFileName, aFileIndex);
}


TInt CSADownloadCandidate::CompareAddressCount(
		const CSADownloadCandidate& aFirst, 
		const CSADownloadCandidate& aSecond)
{
	if (aFirst.AddressCount() == aSecond.AddressCount())
		return 0;

	if (aFirst.AddressCount() > aSecond.AddressCount())
		return -1;
	else
		return 1;
}


TInt CSADownloadCandidate::CompareFileName(
		const CSADownloadCandidate& aFirst, 
		const CSADownloadCandidate& aSecond)
{
	TInt ret = (aFirst.Name()).Compare(aSecond.Name());
	if (ret > 0)
		return 1;
	if (ret < 0)
		return -1;
	return 0;
}