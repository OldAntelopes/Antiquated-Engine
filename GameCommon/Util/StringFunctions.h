#ifndef GAMECOMMON_UTIL_STRING_FUNCTIONS_H
#define GAMECOMMON_UTIL_STRING_FUNCTIONS_H


extern void		GetShortTimeString( uint32 ulTimeSeconds, char* pcOut, BOOL bAccurateSeconds );

extern void		GetApproximateTimeSince( uint32 ulTimeOfEvent, char* pcOut );

extern void		GetNumberStringWithCommas( uint32 ulNumber, char* pcOut );
extern void		GetMoneyStringWithCommas( int nNumber, char* pcOut );

extern void		GetISODateString( uint32 ulUnixTime, char* pcOut );
extern uint32	ReadISODateString( const char* szDate );
extern void		GetDateString( uint32 ulUnixTime, char* pcOut );

#endif

