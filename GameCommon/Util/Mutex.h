#ifndef ANTIQUATED_UTIL_MUTEX_H
#define	ANTIQUATED_UTIL_MUTEX_H

#include "StandardDef.h"

// Very basic mutex implementation.

class Mutex
{
public:
	bool		WaitForMutex( int nTimeoutMS = 2000 )
	{
		int		nWaitTime = 0;
		while ( ( mbMutexLock == true) &&
			    ( nWaitTime < nTimeoutMS ) )
		{
			SysSleep(1);
			nWaitTime += 1;
		}
		if ( nWaitTime >= nTimeoutMS )
		{
			return(false);
		}
		mbMutexLock = true;
		return true;
	}

	void		ReleaseMutex( )
	{
		mbMutexLock = false;
	}

	bool		HasMutex() const { return(mbMutexLock); }

private:

	volatile bool			mbMutexLock = false;
};


#endif