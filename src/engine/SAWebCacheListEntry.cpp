#include "SAWebCacheListEntry.h"

CSAWebCacheListEntry::~CSAWebCacheListEntry()
{
	delete iWebCacheAddress;
}

void CSAWebCacheListEntry::ConstructL(const TDesC8& aWebCacheAddress)
{
	iWebCacheAddress = aWebCacheAddress.AllocL();
}
