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

#include "SADownload.h"
#include "SADownloadCandidate.h"
#include "SADownloadAddress.h"
#include "SAFragment.h"
#include "SAControl.h"
#include "SAProfiler.h"
#include "SADownloadManager.h"
#include "SALogger.h"
#include <f32file.h>
#include <aknnotewrappers.h>

CSADownload::CSADownload() : iState(EWaiting)
{
	iFinishedFragmentsSize = 0;
}

void CSADownload::ConstructL(const CSADownloadCandidate& aDlCandidate, 
							 const RFs& /*aFs*/,
							 const TDesC& aDownloadPath)
{
	iDlCandidate = &const_cast<CSADownloadCandidate&>(aDlCandidate);

	iPath = aDownloadPath.AllocL();
	iName = aDlCandidate.Name().AllocL();
	iHash = aDlCandidate.Hash();
	iSize = aDlCandidate.Size();

	LOG->WriteL(_L("Download size: "));
	LOG->WriteLineL(iSize);

	for (TInt i=0; i<aDlCandidate.AddressCount(); i++)
	{
		CSADownloadAddress* address = aDlCandidate.Address(i);

		AddAddressL(address->Address(), address->Port(), 
			address->FileName(), address->FileIndex());
	}

	// creating fragments
	TUint fragmentSize;

	if (iSize < 1048576) // size < 1MB
		fragmentSize = 102400;
	else
		fragmentSize = iSize / 20;

	TUint processedSize = 0;
	TUint nextSize;
	while (processedSize < iSize-1)
	{
		if ((processedSize + fragmentSize) < iSize)
			nextSize = fragmentSize;
		else
		//	nextSize = -1;
			nextSize = iSize - processedSize-1;

		CSAFragment* fragment = 
			new (ELeave) CSAFragment(processedSize, nextSize, *this);
		CleanupStack::PushL(fragment);
		User::LeaveIfError(iFragments.Append(fragment));
		CleanupStack::Pop();

	/*	LOG->WriteL(_L("Fragment: "));
		LOG->WriteL(processedSize);
		LOG->WriteL(_L(" - "));*/

		if (nextSize < 0)
			processedSize = iSize;
		else
			processedSize += nextSize;

		//LOG->WriteLineL(processedSize);
	}
}


TInt CSADownload::StartDownloadingL(RFs& aFs)
{
	if (iState == EWaiting)
	{		
	/*	HBufC* fileName = HBufC::NewLC(iName->Length());
		TPtr ptr(fileName->Des());
		ptr.Copy(*iName);*/

		LOG->WriteL(_L("Trying to create :"));
		LOG->WriteLineL(*iPath);

		aFs.MkDirAll(*iPath);
		
		RFile file;
		if (file.Replace(aFs, *iPath, EFileRead|EFileShareAny) == KErrNone)
		{
			file.Close();
			iState = EDownloading;
			iDownloadStarted = ETrue;
		}
		else
		{
			file.Close();

		//	CleanupStack::PopAndDestroy(); // fileName
			LOG->WriteLineL(_L(" - Cannot create file!"));
			return KErrGeneral;
		};
		//CleanupStack::PopAndDestroy(); // fileName

		return KErrNone;
	}

	return KErrGeneral;
}


void CSADownload::OnTimerL()
{
	iDownloadingFragmentCount = 0;
	if (iState == EDownloading)
	{
		TInt activeFragmentCount = 0;
		CSAFragment* fragment;		

		for (TInt i=0; i < iFragments.Count(); i++)
		{			
			//iFragments[i]->OnTimerL();
			fragment = iFragments[i];

			/////////////////////////////////////
			// handling finished fragments
			/////////////////////////////////////
			if (fragment->State() == CSAFragment::EFinished)
			{
				iFinishedFragmentsSize += fragment->Size();
				LOG->WriteLineL(_L("Fragment complete!"));
				//iDownloadingFragmentCount--;

				MoveAddressToAvailableFrontL(fragment->Address());

				delete fragment;
				iFragments.Remove(i);
				i--;

				TBool complete = ETrue;
				for (TInt it = 0; it < iFragments.Count(); it++)
					if (iFragments[it]->State() != CSAFragment::EFinished)
					{
						complete = EFalse;
						break;
					}

				if (complete)
				{
					iState = EComplete;
					CTR->DlManager()->HandleDownloadCompletionL();

					
					//TODO Check if mp3
					_LIT8(KMP3,".mp3");
					_LIT8(KTAG,"TAG");
					//if(Name().Right(4).Compare(KMP3) == 0) //skip check as extension is often cheated
						{
						RFile file;
						TInt posTAG = -128; //check TAG
						TInt posGenre = -1;
						TBuf8<3> sTag;
						TBuf8<1> sGenre;
						TUint8 dGenre;
						TInt res = file.Open(CTR->DlManager()->FS(), FullPath(), EFileRead);
						if (res == KErrNone) 
						{
							res=	file.Seek(ESeekEnd, posTAG);
							if (res == KErrNone) 
							{
								User::LeaveIfError(file.Read(sTag,3));
								if(sTag.Compare(KTAG)==0) 
								{
									User::LeaveIfError(file.Seek(ESeekEnd, posGenre));
									User::LeaveIfError(file.Read(sGenre,1));	
									dGenre = sGenre[0];	
									CTR->Profiler()->ProfileGenreL(dGenre);
								}


							}
							file.Close();
						}
					DeleteFragments();
					DeleteAddresses();
					}
					
					if (DlCandidate())
					{
						DlCandidate()->SetDownload(0);
						RemoveDlCandidate();
					}
				}
			}
			else

			/////////////////////////////////////
			// handling failed fragments
			/////////////////////////////////////
			if (fragment->State() == CSAFragment::EFailed)
			{
				LOG->WriteLineL(_L("Fragment failure!"));

				if (fragment->DownloadedSize() > 0)
				{
					// creating a new fragment for the unfinished space					
					TUint startOffset = fragment->StartOffset() + fragment->DownloadedSize();
					TUint fragmentSize = fragment->Size() - fragment->DownloadedSize();

					LOG->WriteL(_L("Creating new fragment: "));					
					LOG->WriteL(startOffset);
					LOG->WriteL(_L(" - "));
					LOG->WriteLineL(startOffset + fragmentSize);

					MoveAddressToAvailableFrontL(fragment->Address());					

					delete fragment;
					iFragments.Remove(i);

					CSAFragment* newFragment = 
						new (ELeave) CSAFragment(startOffset, fragmentSize, *this);
					CleanupStack::PushL(newFragment);
					User::LeaveIfError(iFragments.Insert(newFragment, i));
					CleanupStack::Pop();
					i--;
				}
				else
				{
					if (    (fragment->FailureReason() == CSAFragment::ECannotConnect)
						 || (fragment->FailureReason() == CSAFragment::EHandshakeFailedServiceUnavailable) )
					{
						// removing the wrong address from the list
						TInt index;
						index = iUsedAddresses.Find(fragment->Address());
						if (index >= 0)
						{
							delete iUsedAddresses[index];
							iUsedAddresses.Remove(index);
						}
					}
					else
						MoveAddressToAvailableBackL(fragment->Address());

					//find a new address, and try to download it again
//					CSADownloadAddress* address;
					if (iAvailableAddresses.Count() > 0)
					{
						LOG->WriteLineL(_L("F5!"));
						HBufC* fileName = HBufC::NewLC(iName->Length());
						TPtr ptr(fileName->Des());
						ptr.Copy(*iName);

						LOG->WriteLineL(_L("Starting fragment download at a new address."));
						fragment->StartDownloadingL(CTR->DlManager()->FS(), *(iAvailableAddresses[0]), *iPath);

						iUsedAddresses.Append(iAvailableAddresses[0]);
						iAvailableAddresses.Remove(0);

						CleanupStack::PopAndDestroy(); //fileName
					}
					else
					{
						fragment->SetWaiting();
					//	iState = EFailure;
					}
				}

				/*if (iAvailableAddresses.Count() == 0)
				{
					CTR->DlManager()->HandleDownloadFailureL(this);
				}
				else;*/
			}
			// if (fragment->State() == CSAFragment::EFailed)
			if (fragment->State() == CSAFragment::EDownloading)
			{
				activeFragmentCount++;
				if (fragment->IsDownloaderActive())
					iDownloadingFragmentCount++;
			}
		}


		///////////////////////////////////////
		// starting new downloads
		///////////////////////////////////////
		TInt maxFragments = CTR->DlManager()->MaxFragmentsPerDownload();
		{
			TInt i = 0;
			while ( (i < iFragments.Count()) && 
			        (activeFragmentCount < maxFragments) )
			{
				if (iFragments[i]->State() == CSAFragment::EWaiting)
				{
					if (iAvailableAddresses.Count() > 0)
					{
						HBufC* fileName = HBufC::NewLC(iName->Length());
						TPtr ptr(fileName->Des());
						ptr.Copy(*iName);

						LOG->WriteLineL(_L("Starting fragment download"));
						TInt res = iFragments[i]->StartDownloadingL(CTR->DlManager()->FS(), 
							*(iAvailableAddresses[0]), *iPath);

						CleanupStack::PopAndDestroy(); //fileName

						if (res == KErrNone)
						{
							activeFragmentCount++;

							iUsedAddresses.Append(iAvailableAddresses[0]);
							iAvailableAddresses.Remove(0);
						}
						else
							iFragments[i]->SetFailed(CSAFragment::EGeneral);

						
					}
					else
						break;
				}
				i++;
			}
		}

		if ((iState != EComplete) 
			&& (iAvailableAddresses.Count() == 0) && (activeFragmentCount == 0))
		{
			SetFailedL();
		}

		/*LOG->WriteL(_L("Available addresses:"));
		LOG->WriteLineL(iAvailableAddresses.Count());
		LOG->WriteL(_L("Used addresses:"));
		LOG->WriteLineL(iUsedAddresses.Count());*/
	
		// calling OnTimerL of the fragments
		for (TInt i2=0; i2<iFragments.Count(); i2++)
		{			
			iFragments[i2]->OnTimerL();
		}
	}
}


CSADownload::~CSADownload()
{
	if (iDlCandidate)
		iDlCandidate->SetDownload(0);

	delete iPath;
	delete iName;
	
	DeleteAddresses();
	DeleteFragments();
}


void CSADownload::DeleteAddresses()
{
	iAvailableAddresses.ResetAndDestroy();
	iUsedAddresses.ResetAndDestroy();
}


void CSADownload::AddAddressL(	TUint32 aAddress, 
								TUint aPort, 
								const TDesC8& aFileName, 
								TUint32 aFileIndex )
{
	CSADownloadAddress* address = new (ELeave) CSADownloadAddress;

	CleanupStack::PushL(address);
	User::LeaveIfError(iAvailableAddresses.Insert(address, 0));
	CleanupStack::Pop();

	address->ConstructL(aAddress, aPort, aFileName, aFileIndex);
}


/*void CSADownload::HandleFragmentCompletionL(CSAFragment* aFragment)
{
	iFinishedFragmentsSize += aFragment->Size();
	LOG->WriteLineL(_L("Fragment complete!"));
	//iDownloadingFragmentCount--;

	MoveAddressToAvailableL(aFragment->Address());

	for (TInt i=0; i<iFragments.Count(); i++)
		if (iFragments[i]->State() != CSAFragment::EFinished) return;

	iState = EFinished;
	CTR->DlManager()->HandleDownloadCompletionL();
}*/

void CSADownload::MoveAddressToAvailableFrontL(CSADownloadAddress* aAddress)
{
	TInt index = iUsedAddresses.Find(aAddress);
	if (index >= 0)
	{
		User::LeaveIfError(iAvailableAddresses.Insert(iUsedAddresses[index], 0));
		iUsedAddresses.Remove(index);
	}
}

void CSADownload::MoveAddressToAvailableBackL(CSADownloadAddress* aAddress)
{
	TInt index = iUsedAddresses.Find(aAddress);
	if (index >= 0)
	{
		User::LeaveIfError(iAvailableAddresses.Append(iUsedAddresses[index]));
		iUsedAddresses.Remove(index);
	}
}


/*TSAFragmentDeleted CSADownload::HandleFragmentFailureL(CSAFragment* aFragment)
{
	TSAFragmentDeleted fragmentDeleted = EFragmentNotDeleted;

	if (aFragment->Size() < 0)
	{
	}

	LOG->WriteLineL(_L("Fragment failure!"));
	iDownloadingFragmentCount--;

	if (aFragment->DownloadedSize() > 0)
	{
		// creating a new fragment for the unfinished space
		TInt i = 0;
		for (; i<iFragments.Count(); i++)
			if (iFragments[i] == aFragment) break;

		LOG->WriteL(_L("i: "));
		LOG->WriteLineL(i);

		TUint startOffset = aFragment->StartOffset() + aFragment->DownloadedSize();
		TUint fragmentSize = aFragment->Size() - aFragment->DownloadedSize();

		LOG->WriteL(_L("strtoffset: "));
		LOG->WriteLineL(startOffset);
		LOG->WriteL(_L("fragmentSize: "));
		LOG->WriteLineL(fragmentSize);

		MoveAddressToAvailableL(aFragment->Address());

		fragmentDeleted = EFragmentDeleted;
		delete aFragment;
		iFragments.Remove(i);

		CSAFragment* fragment = 
			new (ELeave) CSAFragment(startOffset, fragmentSize, *this);
		CleanupStack::PushL(fragment);
		User::LeaveIfError(iFragments.Insert(fragment, i));
		CleanupStack::Pop();		

	}
	else
	{
		// removing the wrong address from the list
		TInt index;
		index = iUsedAddresses.Find(aFragment->Address());
		if (index >= 0)
		{
			delete iUsedAddresses[index];
			iUsedAddresses.Remove(index);
		}

		//find a new address, and try to download it again
		CSADownloadAddress* address;
		if (iAvailableAddresses.Count() > 0)
		{
			LOG->WriteLineL(_L("F5!"));
			HBufC* fileName = HBufC::NewLC(iName->Length());
			TPtr ptr(fileName->Des());
			ptr.Copy(*iName);

			LOG->WriteLineL(_L("Starting fragment download at a new address."));
			aFragment->StartDownloadingL(CTR->DlManager()->FS(), *(iAvailableAddresses[0]), *fileName);

			iUsedAddresses.Append(iAvailableAddresses[0]);
			iAvailableAddresses.Remove(0);

			CleanupStack::PopAndDestroy(); //fileName
		}
		else
		{
			iState = EFailure;
		}
	}

	if (iAvailableAddresses.Count() == 0)
	{
		CTR->DlManager()->HandleDownloadFailureL(this);
	}

	return fragmentDeleted;
}*/


void CSADownload::IncreaseDownloadedSizeL(TUint aSize)
{
	iDownloadedSize += aSize;

	CTR->DlManager()->HandleDownloadSizeChangedL(this);
}

void CSADownload::DeleteFragments()
{
	iFragments.ResetAndDestroy();
}

void CSADownload::SetFailedL()
{
	iState = EFailure;
	CTR->DlManager()->HandleDownloadFailureL(this);
}
