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

#include "SALogger.h"
#include <txtetext.h>

_LIT(KLogFileName,"c:\\symella.log.txt");

CSALogger* CSALogger::InstanceL()
{
	CSALogger* instance = static_cast<CSALogger*> 
		( CCoeEnv::Static( KUidSymellaLogger ) );
	
	if (instance == 0)
	{
		instance = new (ELeave) CSALogger();

		CleanupStack::PushL( instance ); 
		instance->ConstructL(); 
		CleanupStack::Pop(); 
	}

	return instance;
}

void CSALogger::ConstructL()
{

#ifdef LOG_TO_FILE
	User::LeaveIfError(iFsSession.Connect());
	if (iLogFile.Open(iFsSession, KLogFileName, EFileWrite) != KErrNone)
		User::LeaveIfError(iLogFile.Create(iFsSession, KLogFileName, EFileWrite));
	else
	{
		TInt pos = 0;
		iLogFile.Seek(ESeekEnd, pos);
	}
#endif

}

CSALogger::~CSALogger()
{
#ifdef LOG_TO_FILE
	iLogFile.Close();
	iFsSession.Close();
#endif
}

void CSALogger::WriteL(const TDesC& aText)
{

#ifdef LOG_TO_FILE
	HBufC8* buf = HBufC8::NewLC(aText.Length());
	TPtr8 ptr(buf->Des());
	ptr.Copy(aText);	
	iLogFile.Write(ptr);
	CleanupStack::PopAndDestroy();
#endif

	if (iObserver)
		iObserver->HandleLogWriteL(aText);
}

void CSALogger::WriteL(const TDesC8& aText)
{

#ifdef LOG_TO_FILE
	iLogFile.Write(aText);
#endif

	if (iObserver)
	{
		HBufC* buf = HBufC::NewLC(aText.Length());
		TPtr ptr(buf->Des());
		ptr.Copy(aText);
		iObserver->HandleLogWriteL(ptr);
		CleanupStack::PopAndDestroy();
	}	
}

void CSALogger::WriteLineL(const TDesC& aText)
{
	WriteL(aText);
	WriteLineL();
}

void CSALogger::WriteLineL(const TDesC8& aText)
{
	WriteL(aText);
	WriteLineL();
}


void CSALogger::WriteLineL()
{

#ifdef LOG_TO_FILE
	iLogFile.Write(_L8("\r\n"));
#endif

	if (iObserver)
	{
		TBuf<1> newLine;
		_LIT(KNewLine, "%c");
		newLine.Format(KNewLine, CEditableText::ELineBreak);
		iObserver->HandleLogWriteL(newLine);
	}
}


void CSALogger::WriteL(TInt aInt)
{
	TBuf<12> buf;
	buf.Num(aInt);
	WriteL(buf);
}


void CSALogger::WriteLineL(TInt aInt)
{
	WriteL(aInt);
	WriteLineL();
}
