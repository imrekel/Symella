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

#include "SADownloadManager.h"
#include "SADownloadCandidate.h"
#include "SADownloadAddress.h"
#include "SAPreferences.h"
#include "SADownload.h"
#include "SALogger.h"
#include "SAStringFormatter.h"

CSADownloadManager::CSADownloadManager() 
 :	iSearchResultsOrder(EByAddressCount)
{
	iMaxSimultaneousDownloads = 3;
	iMaxFragmentsPerDownload = 6;
}


void CSADownloadManager::ConstructL()
{
	User::LeaveIfError(iFs.Connect());
}


CSADownloadManager::~CSADownloadManager()
{
	iDownloads.ResetAndDestroy();
	iDlCandidates.ResetAndDestroy();
	iFs.Close();
}


void CSADownloadManager::ResetHitsList()
{
	iLowestHitCount = 0;
	iTotalHitCount = 0;
	iDlCandidates.ResetAndDestroy();

	if (iSearchObserver)
		iSearchObserver->ResetHitsL();
}


void CSADownloadManager::RegisterQueryHitL( const TDesC8& aFileName, 
											TUint32 aFileSize,
											TUint32 aFileIndex,
											TUint32 aAddress,
											TUint aPort,
											const TSAHash& aHash)
{
	// checks if we have reached the lowest not deletable hit count
	if (iLowestHitCount < PREFERENCES->GetLowestNonDeletableHitCount())
	{
		HBufC8* transformedName = aFileName.AllocLC();
		TPtr8 ptr(transformedName->Des());
		ptr.Fold();

		CSADownloadCandidate* candidate = 0;

		TBool validHash = aHash.IsValid();

		////////////////////////////
	/*	LOG->WriteL(_L("QueryHit: "));
		TInetAddr addr;
		addr.SetAddress(aAddress);
		TBuf<30> buf;
		addr.Output(buf);
		LOG->WriteL(buf);
		LOG->WriteL(_L(":"));
		LOG->WriteL(aPort);
		LOG->WriteL(_L(", size:"));
		LOG->WriteL(aFileSize);
		LOG->WriteL(_L(", index:"));
		LOG->WriteL(aFileIndex);
		LOG->WriteL(_L(", "));
		LOG->WriteLineL(aFileName);*/
		/////////////////////////////

		// searching for a matching file
		for (TInt i=0; i<iDlCandidates.Count(); i++)
		{
			if (validHash && (iDlCandidates[i]->Hash() == aHash))
				candidate = iDlCandidates[i];
			else			
				//TODO: optimalizalni
				if ((iDlCandidates[i]->Size() == aFileSize)  
					&& (iDlCandidates[i]->Name().Compare(*transformedName) == 0))
						candidate = iDlCandidates[i];				 

			if (candidate != 0)
			{										
				TInt addressCount = iDlCandidates[i]->AddressCount() + 1;

				if (iSearchResultsOrder == EByAddressCount)
				// order by address count
				{
					TInt j;
					for (j=0; j<i; j++)
					{
						if ((iDlCandidates[j]->AddressCount() < addressCount))
						{							
							iDlCandidates.Remove(i);
							CleanupStack::PushL(candidate);
							User::LeaveIfError(iDlCandidates.Insert(candidate, j));
							CleanupStack::Pop();

							if (iSearchObserver)
							{
								iSearchObserver->RemoveHitL(i);	
								iSearchObserver->InsertHitL(j, *transformedName, aFileSize, addressCount);
							}
						break;
						}						
					}

					if ((j == i) && iSearchObserver) 
						iSearchObserver->ModifyHitCountL( i, addressCount);	
				}
				else
					if (iSearchObserver)
						iSearchObserver->ModifyHitCountL( i, addressCount);					
				
				break;
			}			
		}
	
		if (candidate == 0)
		// if we haven't found any matching file then create a new entry
		{
			candidate = new (ELeave) CSADownloadCandidate(aFileSize, aHash);			
			CleanupStack::PushL(candidate);
			candidate->ConstructL(*transformedName);

			if (iSearchResultsOrder == EByFileName)
			// order by filename
			{
				TInt i=0;
				while (i < iDlCandidates.Count())
				{
					if (candidate->Name().Compare(iDlCandidates[i]->Name()) < 0)
						break;
					i++;
				}
				
				//if (i>0) i--;
				User::LeaveIfError(iDlCandidates.Insert(candidate, i));
				CleanupStack::Pop(); // candidate

				if (iSearchObserver)
					iSearchObserver->InsertHitL(i, *transformedName, aFileSize, 1);
			}
			else
			{
				User::LeaveIfError(iDlCandidates.Append(candidate));
				CleanupStack::Pop(); // candidate
				
				if (iSearchObserver)
					iSearchObserver->InsertHitL(iDlCandidates.Count()-1, *transformedName, aFileSize, 1);
			}
		}

		candidate->AddAddressL(aAddress, aPort, aFileName, aFileIndex);

		CleanupStack::PopAndDestroy(); // transformedName

		iTotalHitCount++;
		if (iTotalHitCount > PREFERENCES->GetMaxStoredHits())
		{
			RemoveHitWithLowestAddressCount();			
		}
	}
}

void CSADownloadManager::ChangeHitsOrderingL(TSASearchResultsOrder aOrder)
{
	if (iSearchResultsOrder != aOrder)
	{
		iSearchResultsOrder = aOrder;

		switch (aOrder)
		{
			case EByFileName:
			{	
				iSearchObserver->ResetHitsL();
				iDlCandidates.Sort(
					TLinearOrder<CSADownloadCandidate>
					(CSADownloadCandidate::CompareFileName) );

				TBool lastItem = EFalse;
				for (TInt i=0; i<iDlCandidates.Count(); i++)
				{
					if (i == (iDlCandidates.Count() - 1))
						lastItem = ETrue;
					iSearchObserver->InsertHitL(i, iDlCandidates[i]->Name(), 
						iDlCandidates[i]->Size(), iDlCandidates[i]->AddressCount(), lastItem);
				}
					
			}
			break;			

			case EByAddressCount:
			{
				iSearchObserver->ResetHitsL();
				iDlCandidates.Sort(
					TLinearOrder<CSADownloadCandidate>
					(CSADownloadCandidate::CompareAddressCount) );

				TBool lastItem = EFalse;
				for (TInt i=0; i<iDlCandidates.Count(); i++)
				{
					if (i == (iDlCandidates.Count() - 1))
						lastItem = ETrue;
					iSearchObserver->InsertHitL(i, iDlCandidates[i]->Name(), 
						iDlCandidates[i]->Size(), iDlCandidates[i]->AddressCount(), lastItem);
				}
			}
			break;

			default:
			break;
		}
	}
}


void CSADownloadManager::RemoveHitWithLowestAddressCount()
{
	TInt index = -1;

	if (iSearchResultsOrder == EByAddressCount)
		index = iDlCandidates.Count()-1;
	else
	{
		TInt minAddressCount = 1;		
		for (TInt i=0; i<iDlCandidates.Count(); i++)
		{
			if (iDlCandidates[i]->AddressCount() < minAddressCount)
			{
				minAddressCount = iDlCandidates[i]->AddressCount();
				index = i;
			}
		}
	}

	if (index >= 0)
	{
		iLowestHitCount = iDlCandidates[index]->AddressCount();

		if (iLowestHitCount < PREFERENCES->GetLowestNonDeletableHitCount())
		{
			iTotalHitCount -= iDlCandidates[index]->AddressCount();

			delete iDlCandidates[index];
			iDlCandidates.Remove(index);		

			if (iSearchObserver)
				iSearchObserver->RemoveHitL(index);
		}	
	}
		
}


HBufC* CSADownloadManager::CreateHitInfoL(TInt aIndex)
{
	TBuf<400> buf;
	CSADownloadCandidate* hit = iDlCandidates[aIndex];

	// converting text to 16 bit format
	HBufC* nameBuf = HBufC::NewLC(hit->Name().Length());
	TPtr ptr(nameBuf->Des());
	ptr.Copy(hit->Name());

	// checking length
	TPtrC fileName;
	if (nameBuf->Length() < 320)
		fileName.Set(*nameBuf);
	else
		fileName.Set(nameBuf->Left(320));
	
	TBuf<50> fileSize;
	TSAStringFormatter::AppendFileLength(hit->Size(), fileSize);

	buf.Format(_L("%S\n%S\n%d peer(s)"), &fileName, &fileSize, hit->AddressCount());

	CleanupStack::PopAndDestroy();

	return buf.AllocL();
}


HBufC* CSADownloadManager::CreateDownloadStatusInfoL(TInt aPos)
{
	TBuf<400> buf;
	CSADownload* dl = iDownloads[aPos];

	// converting text to 16 bit format
	HBufC* nameBuf = HBufC::NewLC(dl->Name().Length());
	TPtr ptr(nameBuf->Des());
	ptr.Copy(dl->Name());

	// checking length
	TPtrC fileName;
	if (nameBuf->Length() < 320)
		fileName.Set(*nameBuf);
	else
		fileName.Set(nameBuf->Left(320));
	
	TBuf<50> fileSize;
	TSAStringFormatter::AppendFileLength(dl->Size(), fileSize);

	buf.Format(_L("Filename:\n%S\n\nSize: %S\nRemaining fragments: %d\nActive fragments: %d\nUsed addresses: %d\nAvailable addresses: %d"), 
							&fileName, 
							&fileSize,
							dl->FragmentCount(), 
							dl->DownloadingFragmentCount(),
							dl->UsedAddressCount(),
							dl->AvailableAddressCount());

	CleanupStack::PopAndDestroy();

	return buf.AllocL();
}


TSADownloadAttemptResult CSADownloadManager::StartDownloadL(TInt aIndex, TBool aOverWrite)
{
	TSADownloadAttemptResult returnValue = ESuccess;

	if ((aIndex < 0) || (aIndex > iDlCandidates.Count()-1))
		return EFailure;
	CSADownloadCandidate* candidate = iDlCandidates[aIndex];

	if (candidate->GetDownload())
		return EAlreadyDownloading;

	TPtrC destDir(PREFERENCES->GetDestDir());
	
	iFs.MkDirAll(PREFERENCES->GetDestDir());

//	if (!iFs.IsValidName(destDir))
//		return EInvalidPath;

	//checking destination directory
/*	TInt res = iFs.SetSessionPath(destDir);
	if (res != KErrNone)
		return ECannotAccessDirectory;*/

	//checking free space on destination drive
	if (destDir.Length() > 0)
	{
		TChar firstChar = (TChar)destDir[0];
		firstChar.LowerCase();

		if ((firstChar >= 'a') && (firstChar <= 'z'))
		{
			TVolumeInfo info;
			if (iFs.Volume(info, EDriveA + ((char)firstChar - 'a')) != KErrNone)
				return ECannotGetFreeSpace;

			if (info.iFree < TInt64(TUint(candidate->Size())))
				return ENotEnoughFreeSpace;
		}
		else
			return EInvalidPath;
	}
	else
		return EInvalidPath;


	// converting to unicode format
	HBufC* fileName = HBufC::NewLC(candidate->Name().Length());
	TPtr ptr(fileName->Des());
	ptr.Copy(candidate->Name());

	HBufC* fullPath = HBufC::NewLC(fileName->Length() + destDir.Length() + 1);
	ptr.Set(fullPath->Des());
	ptr.Append(destDir);
//	ptr.Append(_L("\\"));
	ptr.Append(*fileName);

	CleanupStack::Pop(2);
	delete fileName;
	CleanupStack::PushL(fullPath);

	// checking if a file with the same name already exits in the target directory
	RFile file;
	TInt res = file.Open(iFs, *fullPath, EFileRead);
	if (res == KErrNone)
	{
		file.Close();

		returnValue = EFileExists;

		LOG->WriteLineL(_L("File already exists!"));


		if (aOverWrite)
		{
			LOG->WriteLineL(_L("Overwriting..."));
			if (iFs.Delete(*fullPath) != KErrNone)
			{
				LOG->WriteLineL(_L("File delete failed!"));
				returnValue = EFailure;
				CleanupStack::PopAndDestroy(); // fullPath
				return ECannotOverwrite;
			}
			else
				returnValue = ESuccess;
		}
		else
		{
			CleanupStack::PopAndDestroy(); // fullPath
			return returnValue;
		}

	}
	else
	{
		// trying to create the file
		res = file.Replace(iFs, *fullPath, EFileRead);
		if (res == KErrNone)
		{
			file.Close();

			if (iFs.Delete(*fullPath) != KErrNone)
			{
				returnValue = ECannotCreateFile;
			}
			else
				returnValue = ESuccess;
		}
		else
			returnValue = ECannotCreateFile;
	}
	
	
	if (returnValue == ESuccess)
	{
		LOG->WriteLineL(_L("returnvalue = ESuccess"));//log
		if (iDownloadObserver)
			iDownloadObserver->InsertDownloadL(iDownloads.Count(), candidate->Name(), 
				candidate->Size(), 0, EDownloadQueued);
		//LOG->WriteLineL(_L("added to downloadobserver"));
		CSADownload* download = new (ELeave) CSADownload;
		CleanupStack::PushL(download);
		download->ConstructL(*(iDlCandidates[aIndex]), FS(), *fullPath);
		LOG->WriteLineL(_L("download constructed"));//log
		iDownloads.Append(download);
		CleanupStack::Pop();
		
		iDlCandidates[aIndex]->SetDownload(download);
	}

	CleanupStack::PopAndDestroy(); // fullPath

	return returnValue;
}

void CSADownloadManager::RemoveDownloadL(TInt aIndex, TBool aDeleteFile)
{
	CSADownload* download = iDownloads[aIndex];
	if (download->State() == CSADownload::EDownloading)
		iActiveDownloadCount--;

	download->DeleteFragments();
	if (aDeleteFile)
		iFs.Delete(download->FullPath());

	delete download;
	iDownloads.Remove(aIndex);

	if (iDownloadObserver)
		iDownloadObserver->RemoveDownloadL(aIndex);
}


void CSADownloadManager::OnTimerL()
{
	// calling OnTimerL of the downloads
	for (TInt i=0; i<iDownloads.Count(); i++)
	{
		iDownloads[i]->OnTimerL();
		/*CSADownload* dl = iDownloads[i];
		TInt per = 0;
		if (dl->Size() > 0)
			per = dl->DownloadedSize()*100 / dl->Size();
		iDownloadObserver->ModifyDownloadL(i, dl->Name(), dl->Size(), dl->ActiveFragments(), per);*/
	}

	// starting the first possible download in the list
	{
		TInt i = 0;
		while ( (i < iDownloads.Count()) && 
				(iActiveDownloadCount <= iMaxSimultaneousDownloads) )
		{
			CSADownload* download = iDownloads[i];
			if (download->State() == CSADownload::EWaiting)
			{
				if (download->StartDownloadingL(iFs) == KErrNone)
				{
					LOG->WriteLineL(_L(" + Starting DL"));
					iActiveDownloadCount++;

					if (iDownloadObserver)
						iDownloadObserver->ModifyDownloadL(i, download->Name(),	download->Size(),
							download->DownloadedSize(), download->DownloadingFragmentCount(), EDownloadInProgress);
				}
				else
				{
					LOG->WriteLineL(_L("Starting DL failed!"));
					download->SetFailedL();
					//iDownloads.Remove(i);
					//i--;
				}
			}
			i++;
		}
	}
}


void CSADownloadManager::HandleDownloadCompletionL()
{
	LOG->WriteLineL(_L("Download complete!"));
	iActiveDownloadCount--;
}


void CSADownloadManager::HandleDownloadFailureL(CSADownload* aDownload)
{
	LOG->WriteLineL(_L("Download: "));
	LOG->WriteL(aDownload->Name());
	LOG->WriteLineL(_L(" failed."));

	aDownload->DeleteFragments();
	aDownload->DeleteAddresses();

	CSADownloadCandidate* dlCandidate = aDownload->DlCandidate();
	if (dlCandidate)
	{
		dlCandidate->SetDownload(0);
		aDownload->RemoveDlCandidate();
	}


	TInt index = iDownloads.Find(aDownload);

	if ( (index >= 0) && (iDownloadObserver) )
	{		
		iDownloadObserver->ModifyDownloadL(index, aDownload->Name(), aDownload->Size(),
				aDownload->DownloadedSize(), aDownload->DownloadingFragmentCount(), EDownloadFailed);
	}

	iActiveDownloadCount--;
}


void CSADownloadManager::HandleDownloadSizeChangedL(CSADownload* aDownload)
{
	TInt index = iDownloads.Find(aDownload);

	//LOG->WriteL(_L("###DlFragmentCount: "));
	//LOG->WriteLineL(aDownload->DownloadingFragmentCount());

	if ( (index >= 0) && (iDownloadObserver) )
	{
		iDownloadObserver->ModifyDownloadL(index, aDownload->Name(),
			aDownload->Size(), aDownload->DownloadedSize(), aDownload->DownloadingFragmentCount(), EDownloadInProgress);
	}
}


TPtrC CSADownloadManager::GetDownloadFileName(TInt aIndex)
{
	return iDownloads[aIndex]->FullPath();
}


CSADownload::TSADownloadState CSADownloadManager::GetDownloadState(TInt aIndex)
{
	return iDownloads[aIndex]->State();
}