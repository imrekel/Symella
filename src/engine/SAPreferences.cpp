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

#include <e32math.h>
#include <badesca.h>
#include <e32std.h>
#include <f32file.h>
#include <in_sock.h>
#include "SAPreferences.h"
#include "SAHostCache.h"
#include "txtetext.h"

const TInt KWebcacheListVersion = 8;

_LIT(KDefaultWebCache, "http://gwc1c.olden.ch.3557.nyud.net:8090/gwc/");

#ifdef EKA2
_LIT(KSettingsFile, "C:\\private\\A0001771\\settings.cfg");
_LIT(KGWebCacheFile, "C:\\private\\A0001771\\gwebcaches.cfg");
_LIT(KHostCacheFile, "C:\\private\\A0001771\\hostcaches.cfg");
#else
_LIT(KSettingsFile, "C:\\system\\apps\\Symella\\settings.cfg");
_LIT(KGWebCacheFile, "C:\\system\\apps\\Symella\\gwebcaches.cfg");
_LIT(KHostCacheFile, "C:\\system\\apps\\Symella\\hostcaches.cfg");
#endif
_LIT8(KPort,"Port");
_LIT8(KDestDir,"DestDir");
_LIT8(KMaxStoredHits,"MaxStoredHits");
_LIT8(KUserId,"UserId");
_LIT8(KEnableProfiling,"EnableProfiling");
_LIT8(KEnableConnectionProfiling,"EnableConnectionProfiling");
_LIT8(KLowestNonDeletableHitCount,"LowestNonDeletableHitCount");
_LIT8(KUseGWC,"UseGWC");
_LIT8(KSettingNetworkConnectionName,"NetworkConnectionName");
_LIT8(KSettingNetworkConnectionId,"NetworkConnectionId");

CSAPreferences* CSAPreferences::InstanceL()
{
	CSAPreferences* instance = static_cast<CSAPreferences*> 
		( CCoeEnv::Static( KUidSymellaPreferences ) );
	
	if (instance == 0)
	{
		instance = new (ELeave) CSAPreferences();

		CleanupStack::PushL( instance ); 
		instance->ConstructL(); 
		CleanupStack::Pop(); 
	}

	return instance;
}

void CSAPreferences::ConstructL()
{
	//create classes
	iDestDir = HBufC::NewL(1);

	User::LeaveIfError(iFsSession.Connect());

	//create dirs if necessary
	iFsSession.MkDirAll(KSettingsFile);
	iFsSession.MkDirAll(KGWebCacheFile);
	iFsSession.MkDirAll(KHostCacheFile);

	//create files if necessary
	CreateDefaultSettingsFileL();
	CreateDefaultHostCacheFileL();
	//CreateDefaultGWebCacheFileL();

	LoadSettingsL();
	if (LoadWebCachesL() != KErrNone)
	{
		CreateDefaultGWebCacheFileL();
		LoadWebCachesL();
	}	
}

CSAPreferences::~CSAPreferences()
{
	iFsSession.Close();
	delete iDestDir;
	delete iLastWebCache;
	delete iAccessPointName;
	
	iWebCaches.ResetAndDestroy();
}

TInt CSAPreferences::LoadSettingsL()
{
	//open file
	RFile file;
	if (file.Open(iFsSession, KSettingsFile, EFileWrite) == KErrNone)
	{
		HBufC8* contentH = HBufC8::NewLC(1000);
		TPtr8 des = contentH->Des();
		file.Read(des);
		TLex8 lex(des);
		TChar ch;
		iEnableProfiling = ETrue;
		iEnableConnectionProfiling = ETrue;
		
		while ((ch = lex.Get()) != 0)
		{
			if (ch == '\r')
			{
				TBuf8<250> row;
				TBuf8<50> name;
				TBuf8<200> value;
				row.Copy(lex.MarkedToken());
				LoadSettingL(row,name,value);
				
				if (name == KPort)
				{
					TLex8 lex2(value);
					if (lex2.Val(iPort) != KErrNone)
						iPort = 6346;
				}
				else
				
				if (name == KDestDir)
				{
					delete iDestDir;
					iDestDir = 0;
					iDestDir = HBufC::NewL(value.Length());
					TPtr destDirPtr(iDestDir->Des());
					destDirPtr.Copy(value);					
				}
				else
				
				if (name == KMaxStoredHits)
				{
					TLex8 lex2(value);
					if (lex2.Val(iMaxStoredHits) != KErrNone)
						iMaxStoredHits = 250;
				}
				else
				
				if (name == KUserId)
				{
					TLex8 lex2(value);
					if (lex2.Val(iUserId) != KErrNone)
						iUserId = -1;
				}
				else
								
				if (name == KLowestNonDeletableHitCount)
				{
					TLex8 lex2(value);
					if (lex2.Val(iLowestNonDeletableHitCount) != KErrNone)
						iLowestNonDeletableHitCount = 3;
				}
				else
				
				if (name == KUseGWC)
				{
					if (value.Left(4).Compare(_L8("true")) == 0)
						iUseGWC = ETrue;
					else iUseGWC = EFalse;
				}
				else
				
				if (name == KEnableProfiling)
				{
					if (value.Left(4).Compare(_L8("true")) == 0)
						iEnableProfiling = ETrue;
					else iEnableProfiling = EFalse;
				}
				else
				
				if (name == KEnableConnectionProfiling)
				{
					if (value.Left(4).Compare(_L8("true")) == 0)
						iEnableConnectionProfiling = ETrue;
					else iEnableConnectionProfiling = EFalse;
				}
				else
				
				if (name == KSettingNetworkConnectionName)
				{
					delete iAccessPointName;
					iAccessPointName = 0;
					iAccessPointName = HBufC::NewL(value.Length());
					TPtr accessPointNamePtr(iAccessPointName->Des());
					accessPointNamePtr.Copy(value);	
						
				}
				else
				
				if (name == KSettingNetworkConnectionId)
				{
					TLex8 lex2(value);
					TInt iapId;
					
					if (lex2.Val(iapId) == KErrNone)
						iAccessPointId = iapId;
					else
						iAccessPointId = -1;
					
					CTR->NetMgr()->SetNetworkConnectionId(iAccessPointId);
				}	
				
				lex.Inc(); //skip \n
				lex.Mark(); //start new row parse
			}
		}
		CleanupStack::PopAndDestroy(); //contentH;
	}
	else
	{
		//set default values
		iPort = 6346;
		
		delete iDestDir;
		iDestDir = 0;
		iDestDir = TPtrC(_L("E:\\Sounds")).AllocL();

		iMaxStoredHits = 250;
		iLowestNonDeletableHitCount = 3;
		iUseGWC = ETrue;
		iEnableProfiling = ETrue;
		iEnableConnectionProfiling = ETrue;
	
	 	TTime now;
	 	now.HomeTime();
	   	TInt64 randSeed = now.Int64();
	     	iUserId = Math::Rand( randSeed );
	     			
	}
	file.Close();
	return KErrNone;
}

TInt CSAPreferences::SaveSettingsL()
{
	//open file
	RFile file;
	if (file.Replace(iFsSession, KSettingsFile, EFileWrite) == KErrNone)
	{
		TBuf8<256> buf;

		buf.Format(_L8("%d"),iPort);
		SaveSettingL(file,KPort,buf);

		HBufC8* buf8 = HBufC8::NewLC(iDestDir->Length());
		TPtr8 ptr(buf8->Des());
		ptr.Copy(*iDestDir);
		SaveSettingL(file,KDestDir, *buf8);
		CleanupStack::PopAndDestroy(); //buf8

		buf.Format(_L8("%d"),iMaxStoredHits);
		SaveSettingL(file,KMaxStoredHits,buf);
		buf.Format(_L8("%d"),iUserId);
		SaveSettingL(file,KUserId,buf);
		buf.Format(_L8("%d"),iLowestNonDeletableHitCount);
		SaveSettingL(file,KLowestNonDeletableHitCount,buf);

		if (iUseGWC)
			SaveSettingL(file,KUseGWC,_L8("true"));
		else
			SaveSettingL(file,KUseGWC,_L8("false"));
		
		if (iEnableProfiling)
			SaveSettingL(file,KEnableProfiling,_L8("true"));
		else
			SaveSettingL(file,KEnableProfiling,_L8("false"));
		
		if (iEnableConnectionProfiling)
			SaveSettingL(file,KEnableConnectionProfiling,_L8("true"));
		else
			SaveSettingL(file,KEnableConnectionProfiling,_L8("false"));	
		
		buf.Copy(AccessPointName());
		SaveSettingL(file, KSettingNetworkConnectionName, buf);
		
		buf.Num(TInt(AccessPointId()));
		SaveSettingL(file, KSettingNetworkConnectionId, buf);
	}

	file.Close();
	return KErrNone;
}

TInt CSAPreferences::SaveSettingL(RFile& aFile, const TDesC8& aName, const TDesC8& aValue)
{
	aFile.Write(aName);
	aFile.Write(_L8(":"));
	aFile.Write(aValue);
	aFile.Write(_L8("\r\n"));
	return KErrNone;
}

TInt CSAPreferences::LoadSettingL(TDesC8& aRow, TDes8& aName, TDes8& aValue)
{
	TInt colon = aRow.Find(_L8(":"));
	if (colon == KErrNotFound)
		return KErrGeneral;
	else
	{
		aName.Copy(aRow.Left(colon));
		aValue.Copy(aRow.Mid(colon+1));
		aName.Trim();
		aValue.Trim();
		return KErrNone;
	}
}

TInt CSAPreferences::LoadHostCachesL(RPointerArray<TSAHostCacheEntry>& aUltrapeers,
						RPointerArray<TSAHostCacheEntry>& aLeafs,
						RPointerArray<TSAHostCacheEntry>& aUnidentifieds,
						RPointerArray<TSAHostCacheEntry>& aWorkingNodes)
{
	aUltrapeers.ResetAndDestroy();
	aLeafs.ResetAndDestroy();
	aUnidentifieds.ResetAndDestroy();
	aWorkingNodes.ResetAndDestroy();

	//open file
	RFile file;
	if (file.Open(iFsSession, KHostCacheFile, EFileRead) == KErrNone)
	{
		HBufC8* contentH = HBufC8::NewLC(1000);
		TPtr8 des = contentH->Des();
		TBuf8<200> buf;
		while (file.Read(buf) == KErrNone)
		{
			if (buf.Length() == 0)
				break;
			des.Append(buf);
			TInt pos;
			while ((pos = des.Find(_L8("\r\n"))) != KErrNotFound)
			{
				TChar ch;
				TLex8 lex(des);
				while ((ch = lex.Peek()) != '\r') lex.Inc();

				//parse row
				TBuf8<150> row;
				row.Copy(lex.MarkedToken());

				TSAHostCacheEntry* entry;
				TUint port = 6346;
				TInt colon = 0;
				TInetAddr addr(INET_ADDR(0,0,0,0));
				addr.SetPort(port);
				TBuf<40> addrs;
						
				colon = row.Find(_L8(":"));
				if (colon != KErrNotFound)
				{
					TBuf8<10> ports;
					TLex8 lex2(row.Mid(colon+1));
					lex2.Val(port);
					addrs.Copy(row.Mid(2,colon-2));
					addr.Input(addrs);
					addr.SetPort(port);
					entry = new (ELeave) TSAHostCacheEntry(addr);
				}
				else
				{
					addr.Copy(row.Mid(2));
					addr.Input(addrs);
					addr.SetPort(6346);
					entry = new (ELeave) TSAHostCacheEntry(addr);
				}
						
				if (row.Left(2).Compare(_L8("W|")) == 0)
					aWorkingNodes.Append(entry);
				if (row.Left(2).Compare(_L8("U|")) == 0)
					aUltrapeers.Append(entry);
				if (row.Left(2).Compare(_L8("L|")) == 0)
					aLeafs.Append(entry);
				if (row.Left(2).Compare(_L8("X|")) == 0)
					aUnidentifieds.Append(entry);

				des.Delete(0,pos+2);
			}
		}

		CleanupStack::PopAndDestroy(); // contentH
	}
	file.Close();
	return KErrNone;
}

TInt CSAPreferences::SaveHostCachesL(RPointerArray<TSAHostCacheEntry>& aUltrapeers,
						RPointerArray<TSAHostCacheEntry>& aLeafs,
						RPointerArray<TSAHostCacheEntry>& aUnidentifieds,
						RPointerArray<TSAHostCacheEntry>& aWorkingNodes)
{
	//open file
	RFile file;
	if (file.Replace(iFsSession, KHostCacheFile, EFileWrite) == KErrNone)
	{
		TBuf<40> ips;
		TBuf8<40> ip8s;
		TBuf8<10> ports;
		TBuf8<150> row;
		TInetAddr ip;
		TInt i;
		for (i=0; i<aUltrapeers.Count(); i++)
		{
			TSAHostCacheEntry* entry = aUltrapeers[i];
			ip.SetAddress(entry->iAddress);
			ip.SetPort(entry->iPort);
			ip.Output(ips);
			ip8s.Copy(ips);
			row.Format(_L8("U|%S:%d\r\n"),&ip8s,ip.Port());
			file.Write(row);
		}
		for (i=0; i<aWorkingNodes.Count(); i++)
		{
			TSAHostCacheEntry* entry = aWorkingNodes[i];
			ip.SetAddress(entry->iAddress);
			ip.SetPort(entry->iPort);
			ip.Output(ips);
			ip8s.Copy(ips);
			row.Format(_L8("W|%S:%d\r\n"),&ip8s,ip.Port());
			file.Write(row);
		}
		for (i=0; i<aLeafs.Count(); i++)
		{
			TSAHostCacheEntry* entry = aLeafs[i];
			ip.SetAddress(entry->iAddress);
			ip.SetPort(entry->iPort);
			ip.Output(ips);
			ip8s.Copy(ips);
			row.Format(_L8("L|%S:%d\r\n"),&ip8s,ip.Port());
			file.Write(row);
		}
		for (i=0; i<aUnidentifieds.Count(); i++)
		{
			TSAHostCacheEntry* entry = aUnidentifieds[i];
			ip.SetAddress(entry->iAddress);
			ip.SetPort(entry->iPort);
			ip.Output(ips);
			ip8s.Copy(ips);
			row.Copy(_L8("X|"));
			row.Append(ip8s);
			row.Format(_L8("X|%S:%d\r\n"),&ip8s,ip.Port());
			file.Write(row);
		}
	}

	file.Close();
	return KErrNone;
}

TInt CSAPreferences::LoadWebCachesL()
{
	iWebCaches.ResetAndDestroy();
	RFile file;
	if (file.Open(iFsSession, KGWebCacheFile, EFileRead) == KErrNone)
	{
		HBufC8* contentH = HBufC8::NewLC(1000);
		TPtr8 des = contentH->Des();
		file.Read(des);
		TLex8 lex(des);
		lex.Mark();
		TChar ch;
		
		if (des.Left(7) == _L8("version"))
		{
			lex.Inc(8);
			lex.Mark();
			
			TInt version = 0; 
			lex.Val(version);
			
			if (version != KWebcacheListVersion)
			{
				file.Close();
				CleanupStack::PopAndDestroy(); //contentH;
				return KErrGeneral;
			}
			
			ch = lex.Get();
			while (ch && (ch != '\n'))
				ch = lex.Get();
				
			if (ch)
				lex.Mark();			
		}
		else
		{
			file.Close();
			CleanupStack::PopAndDestroy(); //contentH;
			return KErrGeneral;
		}
		
		while ((ch = lex.Get()) != 0)
		{
			if (ch == '\r')
			{
				//store the row
				TBuf8<256> row;
				row.Copy(lex.MarkedToken().Left(lex.MarkedToken().Length()-1));
				AddWebCacheL(row, WebCacheCount());
				lex.Inc(); //skip \n
				lex.Mark(); //start new row parse
			}
		}

		CleanupStack::PopAndDestroy(); // contentH
		file.Close();
	}
	else
		return KErrGeneral;
	
	// randomize the starting gwebcache (distribute the load
/*	TTime now;
	now.HomeTime();
	TInt64 seed = now.Int64();
	iGWCPos =  (Math::Rand(seed) % WebCacheCount());*/
	
	iGWCPos = 0;
	
	return KErrNone;
}

void CSAPreferences::MoveWebCacheToTopL(const TDesC8& aWebcache)
{
	for (TInt i=0; i<WebCacheCount(); i++)
	{
		if (WebCache(i)->Address().CompareF(aWebcache) == 0)
		{
			CSAWebCacheListEntry* cache = WebCache(i);
			
			iWebCaches.Remove(i);
			iWebCaches.Insert(cache, 0);
			
			if (iWebCacheListObserver)
				iWebCacheListObserver->WebCacheListChangedL();
			
			break;
		}
	}
}

TInt CSAPreferences::SaveWebCachesL()
{
	RFile file;

	if (file.Replace(iFsSession, KGWebCacheFile, EFileWrite) == KErrNone)
	{
		file.Write(_L8("version:"));
		TBuf8<16> versionBuf;
		versionBuf.Num(KWebcacheListVersion);
		file.Write(versionBuf);
		file.Write(_L8("\r\n"));
		for (TInt i=0; i<WebCacheCount(); i++)
		{
			file.Write(WebCache(i)->Address());
			file.Write(_L8("\r\n"));
		}
	}

	file.Close();
	return KErrNone;
}

TInt CSAPreferences::GetNextWebCacheL(TDes8& aUrl)
{
	iLastWebcacheIndex = -1;
	if (WebCacheCount() < 1)
		aUrl.Copy(KDefaultWebCache);
	else
	{
		if (iGWCPos >= WebCacheCount())
			aUrl.Copy(WebCache(0)->Address());
		else
			aUrl.Copy(WebCache(iGWCPos)->Address());
	
		iLastWebcacheIndex = iGWCPos;
		
		iGWCPos++;
		if (iGWCPos >= WebCacheCount())
			iGWCPos = 0;
	}
	
	delete iLastWebCache;
	iLastWebCache = NULL;
	
	iLastWebCache = aUrl.AllocL();
	
	return iLastWebcacheIndex + 1;
}

const TDesC8& CSAPreferences::GetLastWebCacheL()
{
	return *iLastWebCache;
}

void CSAPreferences::CreateDefaultSettingsFileL()
{
	RFile file;
	if (file.Open(iFsSession, KSettingsFile, EFileWrite) != KErrNone)
	{
		User::LeaveIfError(file.Create(iFsSession, KSettingsFile, EFileWrite));

		//write default settings
		file.Write(KPort);
		file.Write(_L8(":6346\r\n"));

		#ifdef __WINS__
		{
			#ifdef EKA2
				file.Write(_L8("DestDir:C:\\private\\A0001771\\\r\n"));
			#else
				file.Write(_L8("DestDir:C:\\\r\n"));
			#endif
			
		}
		#else
		{	
			TInt res = iFsSession.MkDir(_L("E:\\Downloads\\"));
			if ( (res == KErrNone) || (res == KErrAlreadyExists) )
			{		
				file.Write(_L8("DestDir:E:\\Downloads\\\r\n"));				
			}
			else
			{
				iFsSession.MkDir(_L("C:\\Downloads\\"));
				file.Write(_L8("DestDir:C:\\Downloads\\\r\n"));	
			}
		}
		#endif

		TBuf8<50> buf;
		file.Write(_L8("MaxStoredHits:200\r\n"));
		file.Write(_L8("LowestNonDeletableHitCount:3\r\n"));
		file.Write(_L8("UseGWC:true\r\n"));
	 	TTime now;
	 	now.HomeTime();
	   	TInt64 randSeed = now.Int64();
	     	iUserId = Math::Rand( randSeed );		
		buf.Format(_L8("UserId:%d\r\n"),iUserId);
		file.Write(buf);
		file.Write(_L8("EnableProfiling:false\r\n"));
		file.Write(_L8("EnableConnectionProfiling:false\r\n"));						
	}

	file.Close();
}

void CSAPreferences::CreateDefaultHostCacheFileL()
{
	RFile file;
	if (file.Open(iFsSession, KHostCacheFile, EFileWrite) != KErrNone)
	{
		User::LeaveIfError(file.Create(iFsSession, KHostCacheFile, EFileWrite));

		//let's leave it empty
		//the rows will be in a format 1|ip:port\r\nm where 1 means what type of ip is that
	}

	file.Close();
}

void CSAPreferences::CreateDefaultGWebCacheFileL()
{
	RFile file;
//	if (file.Open(iFsSession, KGWebCacheFile, EFileWrite) != KErrNone)
	{
		User::LeaveIfError(file.Replace(iFsSession, KGWebCacheFile, EFileWrite));
			
		file.Write(_L8("version:"));
		TBuf8<16> versionBuf;
		versionBuf.Num(KWebcacheListVersion);
		file.Write(versionBuf);
		file.Write(_L8("\r\n"));
		file.Write(_L8("http://samnet.dyndns.info/cash/\r\n"));
		file.Write(_L8("http://cache.w3-hidden.cc/\r\n"));
		file.Write(_L8("http://beacon.numberzero.org/gwc.php\r\n"));
		file.Write(_L8("http://www.jdgcs.org/Beacon2/gwc.php\r\n"));
		file.Write(_L8("http://cache3.leite.us/\r\n"));
	}

	file.Close();
}

HBufC* CSAPreferences::GetWebCacheUrls()
{
	TBuf<1> newLine;
	_LIT(KNewLine, "%c");
	newLine.Format(KNewLine, CEditableText::ELineBreak);

	TBuf<1000> bla;
	TBuf<200> temp;
	for (TInt i=0; i<WebCacheCount(); i++)
	{
		temp.Copy(WebCache(i)->Address());
		bla.Append(temp);
		//append newline
		bla.Append(newLine);
	}

	return bla.AllocL();
}

TInt CSAPreferences::SetWebCacheUrlsL(const TDesC& aUrls)
{
	//split the string where there is no alphanumeric char or any specific char
	iWebCaches.ResetAndDestroy();
	
	if (iWebCacheListObserver)
			iWebCacheListObserver->WebCacheListChangedL();

	_LIT(KValidChars,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:.-/~_?&=");
	TBuf<100> validChars;
	validChars.Copy(KValidChars);
	
	TLex lex(aUrls);
	lex.Mark();
	TChar ch;
	while ((ch = lex.Get()) != 0)
	{
		//if we reached a non-alphabetic, blabla char, split the string
		if (validChars.Locate(ch) == KErrNotFound)
		{
			HBufC8* row = HBufC8::NewLC(lex.MarkedToken().Length()-1);
			TPtr8 des = row->Des();
			des.Copy(lex.MarkedToken().Left(lex.MarkedToken().Length()-1));
			AddWebCacheL(*row);

			CleanupStack::PopAndDestroy(); //row

			//skip every unnecessary character
			while ((validChars.Locate(lex.Peek()) == KErrNotFound)&&(lex.Peek() != 0)) lex.Inc();
			lex.Mark();
		}
	}

	//save the gwebcachefile
	SaveWebCachesL();
	return KErrNone;
}

void CSAPreferences::SetDestDir(const TDesC& aDestDir)
{
	delete iDestDir;
	iDestDir = 0;
	
/* 	if ((aDestDir.Length() > 2) && (aDestDir[2] != '\\'))
 	{
 			TInt cut = 0;
		TInt length = aDestDir.Length();
		if (length > 0)
		{
			if (aDestDir[length - 1] == TUint16('\\'))
				cut = 1;

		}
		
			iDestDir = (aDestDir.Left(length - cut)).AllocL();
 	}
 	else*/
 		iDestDir = aDestDir.AllocL();
	
	SaveSettingsL();
}

void CSAPreferences::AddWebCacheL(const TDesC8& aUrl, TInt aIndex)
{
	if (aUrl.Length() > 2)
	{
		TPtrC8 url(aUrl);
		HBufC8* urlBuf = NULL;
		
		_LIT8(KLitHttp, "http://");
		
		if (aUrl.Left(7) != KLitHttp)
		{
			urlBuf = HBufC8::NewLC(aUrl.Length() + 8);
			TPtr8 urlBufPtr(urlBuf->Des());
			urlBufPtr.Append(KLitHttp);
			urlBufPtr.Append(aUrl);
			url.Set(*urlBuf);
		}
		
		TBool foundMatching = EFalse;
		for (TInt i=0; i < WebCacheCount(); i++)
		{
			TPtrC8 address = WebCache(i)->Address();
			if (   (address.CompareF(url) == 0)
				|| (    (address.Right(1) == _L8("/")) 
				    && (address.Left(address.Length()-1).CompareF(url) == 0))
			   )
			{
				foundMatching = ETrue;
				break;
			}
		}
		
		if (!foundMatching)
		{
			CSAWebCacheListEntry* entry = new (ELeave) CSAWebCacheListEntry;
			CleanupStack::PushL(entry);
			
			entry->ConstructL(url);
			User::LeaveIfError(iWebCaches.Insert(entry, aIndex));
			CleanupStack::Pop(); // entry
			
			if (iWebCacheListObserver)
				iWebCacheListObserver->WebCacheListChangedL();
		}
		
		if (urlBuf)
			CleanupStack::PopAndDestroy(); // urlBuf
	}
	
}

void CSAPreferences::SetAccessPointL(const TDesC& aAccessPointName, TUint32 aAccessPointId) 
{
	delete iAccessPointName;
	iAccessPointName = 0;
	iAccessPointName = aAccessPointName.AllocL();
	
	iAccessPointId = aAccessPointId;
	
	SaveSettingsL();
}

void CSAPreferences::RemoveWebCacheL(TInt aIndex)
{
	delete iWebCaches[aIndex];
	iWebCaches.Remove(aIndex);
	
	if (iWebCacheListObserver)
		iWebCacheListObserver->WebCacheListChangedL();
	
	//if (iWebCaches)
}
