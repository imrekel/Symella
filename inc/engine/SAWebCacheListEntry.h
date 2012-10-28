#ifndef SAWEBCACHELISTENTRY_H__
#define SAWEBCACHELISTENTRY_H__

#include <e32base.h>

/**
 * CWebCacheListEntry
 */
class CSAWebCacheListEntry : public CBase
{
public:

	void ConstructL(const TDesC8& aWebCacheAddress);
	
	~CSAWebCacheListEntry();
	
	inline const TDesC8& Address();
	
private:

	HBufC8* iWebCacheAddress;
	
	TInt iErrorCount;
};

inline const TDesC8& CSAWebCacheListEntry::Address() {
	return *iWebCacheAddress;
}

#endif
