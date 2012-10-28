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

#include "SAFragment.h"
#include "SAFragmentDownloader.h"
#include "SAControl.h"
#include "SADownload.h"
#include "SADownloadAddress.h"
#include "SALogger.h"

CSAFragment::CSAFragment(TUint aStartOffset, TUint aSize, CSADownload& aParentDownload) 
: iState(EWaiting),
  iStartOffset(aStartOffset),
  iSize(aSize),
  iParentDownload(aParentDownload)
{
}


CSAFragment::~CSAFragment()
{
	iFile.Close();
	delete iDownloader;
}


TInt CSAFragment::StartDownloadingL(RFs& aFs, 
									const CSADownloadAddress& aAddress, 
									const TDesC& aTarget)
{
//if ((iState == EWaiting) || (iState == EFailed))
//	{
		TInt res;
		/*if ((res = aFs.SetSessionPath(aDestDirectory)) != KErrNone)
			return res;*/

		if ((res = iFile.Open(aFs, aTarget, EFileWrite|EFileShareAny)) != KErrNone)
			return res;
		else
		{
			iAddress = &const_cast<CSADownloadAddress&>(aAddress);		
			TInt fileSize;
			if ((res = iFile.Size(fileSize)) != KErrNone)
				return res;

			/*LOG->WriteL(_L("FileSize: "));
			LOG->WriteLineL(fileSize);
			LOG->WriteL(_L("StartOffset: "));
			LOG->WriteLineL(iStartOffset);
			LOG->WriteL(_L("Size: "));
			LOG->WriteLineL(iSize);*/

			TInt pos;
			// if the file is smaller than the starting position then we enlarge it
			if (fileSize < TInt(iStartOffset))
			{
				pos = 0;
				if ((res = iFile.SetSize(iStartOffset)) != KErrNone)
				{
					LOG->WriteLineL(_L("File SetSize error"));
					return res;
				}				
			}
			
			pos = iStartOffset;
			if ((res = iFile.Seek(ESeekStart, pos)) != KErrNone)
			{
				LOG->WriteLineL(_L("File seek error"));
				return res;
			}
		}

		// issuing download
		TInetAddr localAddr;
		localAddr.SetAddress(0);
		CTR->GetLocalAddress(localAddr);

		TInetAddr remoteAddr;
		remoteAddr.SetAddress(aAddress.Address());
		remoteAddr.SetPort(aAddress.Port());

		iDownloader = new (ELeave) CSAFragmentDownloader(*this, CTR->NetMgr());
		/*if (iSize < 0)
			iDownloader->ConstructL(aAddress.FileIndex(), aAddress.FileName(),
				iStartOffset, -1, localAddr.Address());
		else*/
		iDownloader->ConstructL(aAddress.FileIndex(), aAddress.FileName(),
			iStartOffset, iStartOffset+iSize, localAddr.Address());

		iDownloader->ConnectL(remoteAddr);

		LOG->WriteLineL(_L("Fragmentdownloader created"));

		iState = EDownloading;

		return KErrNone;
//	}

//	return KErrGeneral;
}

void CSAFragment::WriteToFileL(const TDesC8& aBuf)
{
//	LOG->WriteL(aBuf);

	if (iFile.Write(aBuf) != KErrNone)
		LOG->WriteL(_L(" !!! WRITE ERROR"));

	iDownloadedSize += aBuf.Length();
	
	/*LOG->WriteL(_L(" @DL: "));
	LOG->WriteL(iDownloadedSize);*/


	iParentDownload.IncreaseDownloadedSizeL(aBuf.Length());

	// if the fragment is finished
	if (iDownloadedSize >= iSize)
	{		
		iDownloader->CloseL();
		iState = EFinished;
		//iParentDownload.AddAddressL//
	}
}

void CSAFragment::OnTimerL()
{
	if (iDownloader)
	{
		iDownloader->OnTimerL();
		iLastDataSize = iDownloader->LastDataSize();

		if (iState == EFinished)
		{
			delete iDownloader;
			iDownloader = 0;

			//iParentDownload.HandleFragmentCompletionL(this);
		}
		else

		if (iDownloader->State() == CSAFragmentDownloader::EClosing)
		{
			switch (iDownloader->CloseReason())
			{
				case EConnectionFailed:
				{
					if (iDownloader->ConnectionRetryCount() < 1)
					{
						LOG->WriteLineL(_L("Reconnecting..."));
						iDownloader->ReconnectL();

						return;
					}
					else
					{
						/*iState = EWaiting;
						fragmentDeleted = iParentDownload.HandleFragmentFailureL(this);*/

						LOG->WriteLineL(_L("Fragment cannot connect"));
						SetFailed(ECannotConnect);
					}
				}
				break;

				case EHandshakeFailed:
				{
					LOG->WriteLineL(_L("Fragment handshake failed"));
					/*iState = EWaiting;
					fragmentDeleted = iParentDownload.HandleFragmentFailureL(this);*/
					//SetFailed(
					SetFailed(EHandshakeFailed);
				}
				break;

				case EHandshakeFailedFull:
				{				
					LOG->WriteLineL(_L("Servant full"));
					/*iState = EWaiting;
					fragmentDeleted = iParentDownload.HandleFragmentFailureL(this);*/
					//SetFailed(
					SetFailed(EServentFull);
				}
				break;

				case EHandshakeFailedServiceUnavailable:
				{				
					LOG->WriteLineL(_L("Servant full"));
					/*iState = EWaiting;
					fragmentDeleted = iParentDownload.HandleFragmentFailureL(this);*/
					//SetFailed(
					SetFailed(EHandshakeFailedServiceUnavailable);
				}
				break;

				case EDisconnectedWhileDownloading:
				{
					LOG->WriteLineL(_L("Servent disconnected while downloading"));
					/*iState = EWaiting;				
					fragmentDeleted = iParentDownload.HandleFragmentFailureL(this);*/
					SetFailed(EDisconnectedWhileDownloading);
				}
				break;

				default:
				{
					SetFailed(EGeneral);
				}
				break;
			}			
			
			delete iDownloader;
			iDownloader = 0;

			LOG->WriteL(_L("deleted idownloader"));
		}
	}
}

void CSAFragment::SetFailed(TSAFailureReason aReason)
{
	iState = EFailed;
	iFailureReason = aReason;

	iFile.Close();
}

void CSAFragment::SetWaiting()
{
	iState = EWaiting;
}

TBool CSAFragment::IsDownloaderActive()
{
	if (iDownloader)
	{
		if (iDownloader->State() == CSAFragmentDownloader::EGetContent)
			return ETrue;
	}	
	
	return EFalse;
}