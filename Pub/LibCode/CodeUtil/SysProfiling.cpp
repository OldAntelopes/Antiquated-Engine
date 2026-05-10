

#include "StandardDef.h"
#include <vector>
#include <map>

#include "SysProfiling.h"

#define		MAX_SCRIPTEVENT_HASHVAL		128
#define		NUM_ROLLING_AVG_SAMPLES		10

class SysProfileEvent
{
public:
	SysProfileEvent( uint32 ulEventID )
	{
		mpNext = NULL;
		mulEventID = ulEventID;
		mszEventName = NULL;
		mnEventParam = 0;
		mszEventParam = NULL;
		mnTotalLoggedEvents = 0;
		mnNumEventsThisFrame = 0;
		mnRollingEventsCount = 0;
		mfTotalLoggedTime = 0.0f;
		mfLoggedThisFrame = 0.0f;
		mfRollingAverage = 0.0f;
		mfPeakFrameTime = 0.0f;

		mfLastSampleAverage = 0.0f;
		mfLastSamplePeak = 0.0f;
		mnRecentLoggedEvents = 0;
		mfSampleTimer = 0.0f;
	}

	~SysProfileEvent()
	{
		SAFE_FREE( mszEventName );
		SAFE_FREE( mszEventParam );
	}

	void		NewFrame( void  )
	{
		if ( mnNumEventsThisFrame > 0 )
		{
			if ( mfLoggedThisFrame > mfPeakFrameTime )
			{
				mfPeakFrameTime = mfLoggedThisFrame;
			}
			mafRollingTimeSamples[mnRollingEventsCount] = mfLoggedThisFrame;
			mnRollingEventsCount ++;

			if ( mnRollingEventsCount == NUM_ROLLING_AVG_SAMPLES )
			{
			int		nLoop;
				mfRollingAverage = 0.0f;
				for( nLoop = 0; nLoop < NUM_ROLLING_AVG_SAMPLES; nLoop++ )
				{
					mfRollingAverage += mafRollingTimeSamples[nLoop];
				}
				mfRollingAverage /= NUM_ROLLING_AVG_SAMPLES;
				mnRollingEventsCount = 0;
			}
			mfLoggedThisFrame = 0.0f;
			mnNumEventsThisFrame = 0;
		}

		mfSampleTimer += SysGetFrameDelta();
		if ( mfSampleTimer > 1.0f )
		{
			if ( mfPeakFrameTime > mfLastSampleAverage + 5.0f )
			{
				mfPeakFrameTime -= 5.0f;
			}

			if ( mfLastSamplePeak == 0.0f )
			{
				mfLastSamplePeak = mfPeakFrameTime;
			}
			else
			{
				mfLastSamplePeak = (mfLastSamplePeak + mfPeakFrameTime) * 0.5f;
				if ( mfLastSamplePeak > 100.0f )
				{
					mfLastSamplePeak = 100.0f;
				}
			}

			if ( mfLastSampleAverage == 0.0f )
			{
				mfLastSampleAverage = mfRollingAverage;
			}
			else
			{
				mfLastSampleAverage = (mfLastSampleAverage + mfRollingAverage) * 0.5f;
			}		
			mfSampleTimer = 0.0f;
		}

	}
	
	BOOL		IsThisEvent( const char* szEvent, int nParam, const char* szParam )
	{
		if ( ( mnEventParam == nParam ) &&
			 ( tinstricmp( szEvent, mszEventName ) == 0 ) )
		{
			if ( szParam ) 
			{
				if ( ( mszEventParam == NULL ) ||
					 ( tinstricmp( mszEventParam, szParam) != 0 ) )
				{
					return( FALSE );
				}
			}
			else if ( mszEventParam != NULL )
			{
				return( FALSE );
			}
			return( TRUE );
		}
		return( FALSE );
	}

	void		InitEvent( const char* szEventName, int nParam, const char* szEventParam )
	{
		mszEventName = (char*)( SystemMalloc( strlen( szEventName) + 1 ));
		strcpy( mszEventName, szEventName );

		mnEventParam = nParam;
		if ( szEventParam )
		{
			mszEventParam = (char*)( SystemMalloc( strlen( szEventParam) + 1 ));
			strcpy( mszEventParam, szEventParam );		
		}
	}


	void			LogProfileTime( float fMillisecondTime )
	{
		mnTotalLoggedEvents++;
		mnNumEventsThisFrame++;
		mfTotalLoggedTime += fMillisecondTime;
		mfLoggedThisFrame += fMillisecondTime;
	}

	void		MarkStart( int nNestLevel )
	{
		mulLastEventStartTick = SysGetMicrosecondTick();
		mnLastEventNestLevel = nNestLevel;
	}
	

	void		MarkEnd()
	{
		float	fEventProfileTime = ( (float)(SysGetMicrosecondTick() - mulLastEventStartTick) ) * 0.001f;
		LogProfileTime( fEventProfileTime );
	}

	void		GetStats( SYS_PROFILE_STAT* pStatOut )
	{
		if ( mszEventParam )
		{
			sprintf( pStatOut->szEventName, "%s[%s]", mszEventName, mszEventParam );
		}
		else
		{
			strcpy( pStatOut->szEventName, mszEventName );
		}

		if ( ( mnEventParam != 0 ) &&
			 ( mnEventParam != -1 ) )
		{
		char	acString[128];
			sprintf( acString, ",%d", mnEventParam );
			strcat( pStatOut->szEventName, acString );
		}
		pStatOut->fFrameTime = mfLoggedThisFrame;
		pStatOut->fRollingAverageTime = mfRollingAverage;
		if ( ( mfRollingAverage == 0.0f ) &&
			 ( mnTotalLoggedEvents > 0 ) )
		{
			pStatOut->fRollingAverageTime = mfTotalLoggedTime / mnTotalLoggedEvents;
		}

		pStatOut->nNestedLevel = mnLastEventNestLevel;
		pStatOut->fTotalTime = mfTotalLoggedTime;
		pStatOut->nFrameHits = mnNumEventsThisFrame;
		pStatOut->nTotalNumHits = mnTotalLoggedEvents;
		pStatOut->fPeakFrameTime = mfLastSamplePeak;
		pStatOut->fSampleAverageTime = mfLastSampleAverage;
	}

	SysProfileEvent*		GetNext( void ) { return( mpNext ); }
	void					SetNext( SysProfileEvent* pNext ) { mpNext = pNext;}

	uint32		GetID() { return( mulEventID ); }
private:
	SysProfileEvent*		mpNext;
	char*				mszEventName;
	int						mnEventParam;
	char*				mszEventParam;

	uint32	mulEventID;
	int		mnTotalLoggedEvents;
	int		mnNumEventsThisFrame;
	int		mnRollingEventsCount;
	float	mfTotalLoggedTime;
	float	mfLoggedThisFrame;
	float	mfRollingAverage;
	float	mfPeakFrameTime;
	float	mafRollingTimeSamples[NUM_ROLLING_AVG_SAMPLES];
	u64		mulLastEventStartTick;
	int		mnLastEventNestLevel;
	float	mfLastSampleAverage;
	float	mfLastSamplePeak;
	int		mnRecentLoggedEvents;
	float   mfSampleTimer;
};
//---------------------
// Curious hashing structure for managing profile events

std::vector<SysProfileEvent*>	mFrameEventsList;
std::vector<SysProfileEvent*>	mLastFrameEventsList;

SysProfileEvent*		mapSysProfileEventsHashMap[MAX_SCRIPTEVENT_HASHVAL] = { NULL };
uint32				msulNextProfileEventID = 1000;

int				msnSysProfileNestLevel = 0;

//------------------------------------------------------------------------
//--------------------------------------------

int			SysProfileGetEventHash( const char* pcEventName, int nParam )
{
int		nValue = *pcEventName;

	nValue += (pcEventName[1] * 11) + nParam;
	return( nValue % MAX_SCRIPTEVENT_HASHVAL);
}

SysProfileEvent*		SysProfileGetEventFromID( u64 ullEventID )
{
int		nHash = (int)( ullEventID >> 32 );
uint32		ulEventID = (ullEventID & 0xFFFFFFFF);
SysProfileEvent*		pEvents = mapSysProfileEventsHashMap[nHash];

	while( pEvents )
	{
		if ( pEvents->GetID() == ulEventID )	
		{
			return( pEvents );
		}
		pEvents = pEvents->GetNext();
	}
	return( NULL );
}

u64		SysProfileGetEventID( int nHash, SysProfileEvent* pEvent )
{
u64 ullHash = ((u64)nHash) << 32;
	ullHash |= pEvent->GetID();
	return( ullHash );
}

SysProfileEvent*		SysProfileGetEvent( const char* pcEventName, int nParam, const char* szParam, u64* pullOut )
{
int		nHash = SysProfileGetEventHash( pcEventName, nParam );
SysProfileEvent*		pEvents = mapSysProfileEventsHashMap[nHash];

	while( pEvents )
	{
		if ( pEvents->IsThisEvent( pcEventName, nParam, szParam) == TRUE )
		{
			if ( pullOut )
			{
				*pullOut = SysProfileGetEventID( nHash, pEvents );
			}
			return( pEvents );
		}

		pEvents = pEvents->GetNext();
	}
	return( NULL );
}

SysProfileEvent*		SysProfileAddEvent( const char* pcEventName, int nEventParam, const char* szEventParam, u64* pullOut )		// pullOut haha, hungarian joke.
{
int		nHash = SysProfileGetEventHash( pcEventName, nEventParam );
SysProfileEvent*		pEvent = new SysProfileEvent( msulNextProfileEventID++ );

	pEvent->SetNext( mapSysProfileEventsHashMap[nHash] );
	mapSysProfileEventsHashMap[nHash] = pEvent;

	pEvent->InitEvent( pcEventName, nEventParam, szEventParam );

	if ( pullOut )
	{
		*pullOut = SysProfileGetEventID( nHash, pEvent );
	}
	return(pEvent);
}

void	SysProfileReleaseAll()
{
SysProfileEvent* pNext;
SysProfileEvent* pEvent;

	for( int i = 0; i < MAX_SCRIPTEVENT_HASHVAL; i++ )
	{
		pEvent = mapSysProfileEventsHashMap[i];
		while( pEvent )
		{
			pNext = pEvent->GetNext();
			delete( pEvent );
			pEvent = pNext;
		}
		mapSysProfileEventsHashMap[i] = NULL;
	}
}
//------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------
// External interface functions
//------------

//--------
// SysProfileNewFrame : Called at the start of each frame to reset per-frame stats
//
void		SysProfileNewFrame( void )
{
int		nHashLoop;
SysProfileEvent*		pEvent;

	msnSysProfileNestLevel = 0;
	mLastFrameEventsList = mFrameEventsList;
	mFrameEventsList.clear();

	for( nHashLoop = 0; nHashLoop < MAX_SCRIPTEVENT_HASHVAL; nHashLoop++ )
	{
		pEvent = mapSysProfileEventsHashMap[nHashLoop];

		while( pEvent )
		{
			pEvent->NewFrame();
			pEvent = pEvent->GetNext();
		}
	}
}

//---------
// SysProfileStartEvent : Marks the start of a profiling event
//
u64		SysProfileStartEvent( const char* szEventName, int nEventParam, const char* szEventParam )
{
u64		ullEventID;
SysProfileEvent* pEvent;

	pEvent = SysProfileGetEvent( szEventName, nEventParam, szEventParam, &ullEventID);
	if ( pEvent == NULL) 
	{
		pEvent = SysProfileAddEvent( szEventName, nEventParam, szEventParam, &ullEventID);
	}

	if ( pEvent )
	{
		mFrameEventsList.push_back( pEvent );
		pEvent->MarkStart( msnSysProfileNestLevel++ );
		return( ullEventID );
	}
	return( 0 );
}

//-------
// SysProfileEndEvent : Marks the end of a profiling event
//
void		SysProfileEndEvent( u64 ullEventID )
{
	SysProfileEvent* pEvent = SysProfileGetEventFromID( ullEventID );

	if ( pEvent )
	{
		msnSysProfileNestLevel--;
		pEvent->MarkEnd();
	}
}

void	SysProfileNextEvent( u64* pullEventIDInOut, const char* szEventName )		// Convenience func for events on same level ; Calls end then start
{
	SysProfileEndEvent( *pullEventIDInOut );
	*pullEventIDInOut = SysProfileStartEvent( szEventName );
} 

//--------------------
// SysProfileGetStat : Retrieves profiling stats for the given stat number
//
BOOL		SysProfileGetStat( int nStatNum, SYS_PROFILE_STAT* pStatOut )
{
	if ( nStatNum < (int)mLastFrameEventsList.size() )
	{
		mLastFrameEventsList[nStatNum]->GetStats( pStatOut );
		return( TRUE );
	}
	return( FALSE );

}
