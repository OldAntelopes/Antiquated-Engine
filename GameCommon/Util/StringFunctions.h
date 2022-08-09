#ifndef GAMECOMMON_UTIL_STRING_FUNCTIONS_H
#define GAMECOMMON_UTIL_STRING_FUNCTIONS_H


extern void		GetShortTimeString( ulong ulTimeSeconds, char* pcOut, BOOL bAccurateSeconds );

extern void		GetApproximateTimeSince( ulong ulTimeOfEvent, char* pcOut );

extern void		GetNumberStringWithCommas( ulong ulNumber, char* pcOut );
extern void		GetMoneyStringWithCommas( int nNumber, char* pcOut );

extern void		GetISODateString( ulong ulUnixTime, char* pcOut );
extern ulong	ReadISODateString( const char* szDate );
extern void		GetDateString( ulong ulUnixTime, char* pcOut );

#endif

