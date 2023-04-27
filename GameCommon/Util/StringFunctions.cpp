
#include <stdio.h>
#include <time.h>
#include "StandardDef.h"

#include "StringFunctions.h"

extern void		GetISODateString( uint32 ulUnixTime, char* pcOut );
extern uint32	ReadISODateString( const char* szDate );

void		GetApproximateTimeSince( uint32 ulTimeOfEvent, char* pcOut )
{
uint32		ulSystemTime = SysGetTimeLong();
uint32		ulTimeGap;

	if ( ulSystemTime > ulTimeOfEvent )
	{
		ulTimeGap = ulSystemTime - ulTimeOfEvent;

		if ( ulTimeGap > (48*60*60) )
		{
		int	nDays = ulTimeGap / (24*60*60);
			
			sprintf( pcOut, "%dd ago", nDays );
		}
		else if ( ulTimeGap > (2*60*60) )
		{
		int		nHours = ulTimeGap / (60*60);
		
			sprintf( pcOut, "%dh ago", nHours );
		}
		else if ( ulTimeGap > 60 )
		{
		int		nMins = ulTimeGap / 60;

			sprintf( pcOut, "%dm ago", nMins );
		}
		else if ( ulTimeGap > 50 )
		{
			sprintf( pcOut, "50s ago" );
		}
		else if ( ulTimeGap > 40 )
		{
			sprintf( pcOut, "40s ago" );
		}
		else if ( ulTimeGap > 30 )
		{
			sprintf( pcOut, "30s ago" );
		}
		else if ( ulTimeGap > 15 )
		{
			sprintf( pcOut, "just now" );
		}
		else
		{
			pcOut[0] = 0;
		}
	}
	else
	{
		pcOut[0] = 0;
	}
}

void		GetShortTimeString( uint32 ulTimeSeconds, char* pcOut, BOOL bAccurateSeconds )
{
	if ( ulTimeSeconds >= (24*60*60) )
	{
	int		nDays;
	int		nHours;

		nDays = ulTimeSeconds / (24*60*60);
		nHours = (ulTimeSeconds - (nDays * (24*60*60)))  / (60*60);

		if ( nDays == 1 )
		{
			if ( nHours == 0 )
			{
				sprintf( pcOut, "%d day", nDays );
			}
			else
			{
				sprintf( pcOut, "%d day %d hrs", nDays, nHours );
			}
		}
		else
		{
			if ( nHours == 0 )
			{
				sprintf( pcOut, "%d days", nDays );
			}
			else
			{
				sprintf( pcOut, "%d days %d hrs", nDays, nHours );
			}
		}
	}
	else if ( ulTimeSeconds >= (60*60) )
	{
	int		nHours;
	int		nMins;

		nHours = ulTimeSeconds / (60*60);
		nMins = (ulTimeSeconds - (nHours * (60*60))) / 60;

		if ( nMins == 0 )
		{
			sprintf( pcOut, "%d hr", nHours );
		}
		else
		{
			sprintf( pcOut, "%d hr %d mins", nHours, nMins );
		}
	}
	else if ( ulTimeSeconds >= (60*2) )
	{
	uint32	ulNumSeconds = (ulTimeSeconds % 60 );

		if ( ( bAccurateSeconds ) &&
			 ( ulNumSeconds != 0 ) )
		{
			sprintf( pcOut, "%d mins %d sec", (int)( ulTimeSeconds / 60 ), (int)(ulTimeSeconds % 60) );
		}
		else
		{
			sprintf( pcOut, "%d mins", (int)( ulTimeSeconds / 60 ) );
		}
	}
	else if ( ulTimeSeconds >= (60) )
	{
	uint32	ulNumSeconds = (ulTimeSeconds % 60 );

		if ( ( bAccurateSeconds ) &&
			 ( ulNumSeconds != 0 ) )
		{
			sprintf( pcOut, "1 minute %d sec", (int)ulNumSeconds );
		}
		else
		{
			sprintf( pcOut, "1 minute" );
		}
	}
	else if ( ulTimeSeconds > (45) )
	{
		if ( bAccurateSeconds )
		{
			sprintf( pcOut, "%d sec", (int)ulTimeSeconds );
		}
		else
		{
			sprintf( pcOut, "under a minute" );
		}
	}
	else
	{
		sprintf( pcOut, "%d sec", (int)ulTimeSeconds );
	}
}


void		GetNumberStringWithCommas( uint32 ulNumber, char* pcOut )
{
	if ( ulNumber >= 1000000000 )
	{
		sprintf( pcOut, "%ld,%03ld,%03ld,%03ld", ulNumber / 1000000000, (ulNumber % 1000000000)/1000000, (ulNumber % 1000000)/1000, ulNumber % 1000 );	
	}
	else if ( ulNumber >= 1000000 )
	{
		sprintf( pcOut, "%ld,%03ld,%03ld", ulNumber / 1000000, (ulNumber % 1000000)/1000, ulNumber % 1000 );
	}
	else if ( ulNumber >= 1000 )
	{
		sprintf( pcOut, "%ld,%03ld", ulNumber / 1000, ulNumber % 1000 );
	}
	else
	{
		sprintf( pcOut, "%ld", ulNumber );
	}

}


void		GetMoneyStringWithCommas( int nNumber, char* pcOut )
{
char	acBuff[256];
	GetNumberStringWithCommas( (uint32)(nNumber), acBuff );
	sprintf( pcOut, "$%s", acBuff );
}

void		GetISODateString( uint32 ulUnixTime, char* pcOut )
{
SYS_LOCALTIME		xLocalTime;

	SysGetLocalTimeFromUnixTimestamp( ulUnixTime, &xLocalTime );
	sprintf( pcOut, "%d-%02d-%02dT%02d:%02d:%02d", xLocalTime.wYear, xLocalTime.wMonth, xLocalTime.wDay, xLocalTime.wHour, xLocalTime.wMinute, xLocalTime.wSecond );
} 

uint32	ReadISODateString( const char* szDate )
{
SYS_LOCALTIME		xLocalTime;
char		acBuff[256];
char*		pcYear;
char*		pcMonth;
char*		pcDay;
char*		pcHour;
char*		pcMinute;
char*		pcSec;
uint32		ulTimestamp;

	strcpy( acBuff, szDate );
	pcYear = acBuff;
	pcMonth = SplitStringSep( pcYear, '-' );
	pcDay = SplitStringSep( pcMonth, '-' );
	pcHour = SplitStringSep( pcDay, 'T' );
	pcMinute = SplitStringSep( pcHour, ':' );
	pcSec = SplitStringSep( pcMinute, ':' );

	xLocalTime.wYear = (short)strtol( pcYear, NULL, 10 );
	xLocalTime.wMonth = (short)strtol( pcMonth, NULL, 10 );
	xLocalTime.wMinute = (short)strtol( pcMinute, NULL, 10 );
	xLocalTime.wDay = (short)strtol( pcDay, NULL, 10 );
	xLocalTime.wHour = (short)strtol( pcHour, NULL, 10 );
	xLocalTime.wSecond = (short)strtol( pcSec, NULL, 10 );

	ulTimestamp = SysGetUnixTimestampFromLocalTime( &xLocalTime );
	return( ulTimestamp );
}

const char*		acMonthNames[12] = 
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void		GetDateString( uint32 ulUnixTime, char* pcOut )
{
SYS_LOCALTIME		xLocalTime;

	SysGetLocalTimeFromUnixTimestamp( ulUnixTime, &xLocalTime );

	if ( xLocalTime.wHour < 12 )
	{
	int		nHour = xLocalTime.wHour;
		if ( nHour == 0 ) nHour = 12;
		sprintf( pcOut, "%d %s %d  %d:%02dam", xLocalTime.wDay, acMonthNames[ xLocalTime.wMonth ], xLocalTime.wYear + 1900, nHour, xLocalTime.wMinute );
	}
	else
	{
	int		nHour = xLocalTime.wHour - 12;
		if ( nHour == 0 ) nHour = 12;
		sprintf( pcOut, "%d %s %d  %d:%02dpm", xLocalTime.wDay, acMonthNames[ xLocalTime.wMonth ], xLocalTime.wYear + 1900, nHour, xLocalTime.wMinute );
	}
}
