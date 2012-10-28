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

#ifndef SADOWNLOADADDRESS_H
#define SADOWNLOADADDRESS_H

#include <e32base.h>


/**
 */
class CSADownloadAddress : public CBase
{
public:

	~CSADownloadAddress();

	void ConstructL(TUint32 aAddress, 
					TUint aPort, 
					const TDesC8& aFileName, 
					TUint32 aFileIndex);

	inline TUint32 Address() const;

	inline TUint Port() const;

	inline TPtrC8 FileName() const;

	inline TUint32 FileIndex() const;

private:

	/**
	 * IP address
	 */
	TUint32		iAddress;

	TUint		iPort;

	HBufC8*		iFileName;

	TUint32		iFileIndex;
};


inline TUint32 CSADownloadAddress::Address() const {
	return iAddress;
}

inline TUint CSADownloadAddress::Port() const {
	return iPort;
}

inline TPtrC8 CSADownloadAddress::FileName() const {
	return *iFileName;
}

inline TUint32 CSADownloadAddress::FileIndex() const {
	return iFileIndex;
}

#endif
