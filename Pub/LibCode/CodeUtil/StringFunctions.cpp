
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>

#include "StandardDef.h"


char	macDateBuffer[64] = "";


const char*	maszMonthNames[ 12 ] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug","Sep","Oct", "Nov", "Dec" 
};

/***************************************************************************
 * Function    : GetMonthName
 * Params      : a month 1-12
 * Returns     :
 * Description : Returns the string for the month [Kim]
 ***************************************************************************/
const char* GetMonthName(int nMonth)
{
int nCheckedMonth;

	nCheckedMonth = (nMonth<1) ? 1 : nMonth;
	nCheckedMonth = (nCheckedMonth>12) ? 12 : nCheckedMonth;
	
	return maszMonthNames[nCheckedMonth-1];
}



//Mit this one should be in a more generic file, but I dunno where to put it :)
//		Aye :] Globals\String Functions.c sounds good to me - M
//
// [This comment conversation snippet (c) 2002-odd so am leaving here for posterity alone)

/***************************************************************************
 * Function    : SplitStringSep - shamelessly stolen and extended from Mit's :]
 * Params      : Char pointer to string to be split, char which will be separator
 * Returns     : Pointer to second 'word' of the string.
 * Description : Given a string of the format "xxxxxZyyy" and a separator char 'Z',
 *				 this function returns the pointer to "yyy", null terminates
 *				 the "xxxxx" part and eat the Z char as his meal.
 ***************************************************************************/
char* SplitStringSep( char* pszOriginal, char cCutHere )
{
char*	pcString;

	pcString = pszOriginal;

	while ( ( *(pcString) != cCutHere ) &&
		    ( *(pcString) != 0   ) )
	{
		pcString++;
	}

	if ( *(pcString) != 0 )
	{
		*(pcString) = 0;
		pcString++;
	}
	
	return( pcString );
}

const char*	FindChar( const char* pcRunner, char cChar )
{
	while ( *pcRunner != 0 )
	{
		if ( *pcRunner == cChar )
		{
			return( pcRunner );
		}
		pcRunner++;
	}

	return( NULL );
}


void		StringToLower( const char* pcSourceString, char* pcOutString )
{
int		nLoop;
int		nLen = strlen( pcSourceString );
char*		pcOutRunner = pcOutString;
const char*		pcRunner = pcSourceString;

    for ( nLoop = 0; nLoop < nLen; nLoop++ )
	{
		*pcOutRunner = tolower( *pcRunner );
		pcOutRunner++;
		pcRunner++;
    }
}

char*	ScriptGetNextLineEx( const char* pcText, char* szOutput, int nOutputBufferSize, const char* pcScriptEnd )
{
int		nLoop = 0;
char*	pcPos = (char*)pcText;
	
	while ( ( *pcPos != '\n' ) &&
		    ( *pcPos != '\r' ) &&
			( pcPos < pcScriptEnd ) )
	{
		nLoop++;
		if ( nLoop == (nOutputBufferSize-1) )
		{
//			PANIC_IF(TRUE,"Script line too long" );
			return( (char*)pcScriptEnd );
		}
		pcPos++;
	}

	if ( pcPos >= pcScriptEnd )
	{
		nLoop = 0;
		while ( pcText + nLoop != pcPos )
		{
			szOutput[nLoop] = pcText[nLoop];
			nLoop++;
		}
		szOutput[nLoop] = 0;
		return( (char*)pcScriptEnd );
	}
	if ( *pcPos == '\r' )
	{
		*pcPos = 0;
		strcpy( szOutput, pcText );
		*pcPos = '\r';
		pcPos++;
		pcPos++;
		return( pcPos );
	}
	else
	{
		*pcPos = 0;
		strcpy( szOutput, pcText );
		*pcPos = '\n';
		pcPos++;
		return( pcPos );
	}
}


char*	ScriptGetNextLine( const char* pcText, char* szOutput, const char* pcScriptEnd )
{
	return( ScriptGetNextLineEx( pcText, szOutput, 256, pcScriptEnd ) );
}

int StripWhiteSpaceLeft( char* pcLine )
{
char*	pcStart = pcLine;
int		nNumCharsRemoved = 0;

	while ( ( *pcStart == ' ' ) ||
			( *pcStart == '\r' ) ||
			( *pcStart == '\n' ) ||
			( *pcStart == '\t' ) )
	{
		pcStart++;
		nNumCharsRemoved++;
	}
	if ( pcStart != pcLine )
	{
		while ( *pcStart != 0 )
		{
			*pcLine++ = *pcStart++;
		}
		*pcLine = 0;
	}
	return( nNumCharsRemoved );
} 

int		StringExpandTabs( const char* pcLine, char* pcExpandedOut )
{
const char*		pcRunner = pcLine;
char*			pcOutRunner = pcExpandedOut;
int			nStrLen = 0;

	while ( *pcRunner != 0 )
	{
		if ( *pcRunner == '\t' )
		{
			*pcExpandedOut = ' ';
			pcExpandedOut++;
			*pcExpandedOut = ' ';
			pcExpandedOut++;
			*pcExpandedOut = ' ';
			pcExpandedOut++;
			*pcExpandedOut = ' ';
			pcExpandedOut++;
			nStrLen += 4;
		}
		else
		{
			*pcExpandedOut = *pcRunner;
			pcExpandedOut++;
			nStrLen++;
		}
		pcRunner++;
	}
	*pcExpandedOut = 0;
	return( nStrLen );
}

void StripWhiteSpaceRight( char* pcLine )
{
int		nLen;
	nLen = strlen( pcLine ) - 1;
	while ( ( pcLine[nLen] == ' ' ) ||
			( pcLine[nLen] == '\r' ) ||
			( pcLine[nLen] == '\n' ) ||
			( pcLine[nLen] == '\t' ) )
	{
		pcLine[nLen] = 0;
		nLen--;
		if ( nLen == 0 ) return;
	}
} 

void	SysGetLogDateTime( char* szOutBuffer )
{
const char*		aszMonthNames[ 12 ] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug","Sep","Oct", "Nov", "Dec" };
#ifdef _WIN32
__time32_t          td; 
struct          tm *dcp; 

	_time32(&td); 
	dcp = _localtime32(&td); 
	sprintf( szOutBuffer, "%d-%s-%d|%02d:%02d:%02d", dcp->tm_mday, aszMonthNames[dcp->tm_mon], dcp->tm_year+1900, dcp->tm_hour, dcp->tm_min, dcp->tm_sec);
#else
time_t now = time(NULL);
struct tm* local_tm = localtime(&now);

	sprintf( szOutBuffer, "%d-%s-%d|%02d:%02d:%02d", local_tm->tm_mday, aszMonthNames[local_tm->tm_mon], local_tm->tm_year+1900, local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec );

#endif

}

u64		tinstrtou64( const char* szVal, char** delimiter, int nBase )
{
#ifdef _WIN32
	return( _strtoi64( szVal, delimiter, nBase ) );
#else
	return( strtoll( szVal, delimiter, nBase ) );
#endif
}

char* GetRealTimeDate( ulong ulTimeSeconds, BOOL bIncludeClockTime )
{
#ifdef _WIN32
struct tm *dcp;

	dcp = _localtime32((__time32_t*)(&ulTimeSeconds) );
	if ( dcp != NULL )
	{
		if ( bIncludeClockTime )
		{
			sprintf( macDateBuffer, "%d %s %d - %02d:%02d", dcp->tm_mday, GetMonthName(dcp->tm_mon + 1), dcp->tm_year+1900, dcp->tm_hour, dcp->tm_min );
		}
		else
		{
			sprintf( macDateBuffer, "%d %s %d", dcp->tm_mday, GetMonthName(dcp->tm_mon + 1), dcp->tm_year+1900 );
		}
	}
#else
	time_t now = time(NULL);
	struct tm* local_tm = localtime(&now);

	if ( bIncludeClockTime )
	{
		sprintf( macDateBuffer, "%d %s %d - %02d:%02d", local_tm->tm_mday, GetMonthName(local_tm->tm_mon + 1), local_tm->tm_year+1900, local_tm->tm_hour, local_tm->tm_min );
	}
	else
	{
		sprintf( macDateBuffer, "%d %s %d", local_tm->tm_mday, GetMonthName(local_tm->tm_mon + 1), local_tm->tm_year+1900 );
	}
#endif
	return( macDateBuffer );
}

//----------------------------------
// tinstricmp
//	returns  0 if the text matches
//  -1 if not
//  case insensitive
//----------------------------------
int		tinstricmp( const char* szText, const char* szFind )
{
int		nLoop = 0;
char	cChar1;
char	cChar2;

	while ( szText[nLoop] != 0 )
	{
		cChar1 = szFind[nLoop];
		cChar2 = szText[nLoop];
		if ( cChar1 >= 'a' && cChar1 <= 'z' )
		{	
			cChar1 += ('A' - 'a');
		}
		if ( cChar2 >= 'a' && cChar2 <= 'z' )
		{	
			cChar2 += ('A' - 'a');
		}
		if ( cChar1 != cChar2 )
		{
			break;
		}
		nLoop++;
	}
	if ( ( szText[nLoop] == 0 ) &&
		 ( szFind[nLoop] == 0 ) )
	{
		return( 0 );
	}
	return( -1 );
}


//----------------------------------
// tinstrcmp
//	returns  0 if the text matches
//  -1 if not
//	case sensitive
//----------------------------------
int		tinstrcmp( const char* szText, const char* szFind )
{
int		nLoop = 0;

	while ( ( szText[nLoop] != 0 ) &&
			( szFind[nLoop] == szText[nLoop] ) )
	{
		nLoop++;
	}
	if ( ( szText[nLoop] == 0 ) &&
		 ( szFind[nLoop] == 0 ) )
	{
		return( 0 );
	}
	return( -1 );
}
