#ifndef UNIVERSAL_CODEUTIL_H
#define UNIVERSAL_CODEUTIL_H

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage, coz in yr 2000 it was just the way, aight
{
#endif
	
#ifndef BASETYPES
#ifndef MIT_TYPES
typedef unsigned short			ushort;
typedef unsigned char			uchar;
typedef unsigned int			uint;
typedef unsigned long long		u64;
typedef int			            BOOL;
#endif
#endif
	
#ifndef UINT32_DEFINED
typedef unsigned __int32		uint32;
#define UINT32_DEFINED
#endif

/**********************************************
 *****   Universal CodeUtil Library      ******
 **********************************************
 **                                          **
 **	This library contains various (somewhat  **
 ** random) useful utility functions, such   **
 ** as some custom string manipulation stuff **
 **	that is used 'globally' throughout 		 **
 ** the Universal codebase and the 'public'  **
 ** libraries								 **
 **											 **
 **  (c) 2014 Mit. http://theuniversal.net   **
 **********************************************/

extern unsigned int	BasicHash(const char* szString );


// ---------------------- Miscellaneous string manipulation functions


extern char* SplitStringSep( char* pszOriginal, char cCutHere );

extern const char*	FindChar( const char* pcRunner, char cChar );


extern char*	ScriptGetNextLine( const char* pcText, char* szOutput, const char* pcScriptEnd );
extern char*	ScriptGetNextLineEx( const char* pcText, char* szOutput, int nOutputBufferSize, const char* pcScriptEnd );

extern void		StripWhiteSpaceRight( char* pcLine );
extern int		StripWhiteSpaceLeft( char* pcLine );
extern int		StringExpandTabs( const char* pcLine, char* pcExpandedOut );
extern void		StringToLower( const char* pcText, char* pcTextOut );

extern char*	GetRealTimeDate( uint32 ulTimeSeconds, BOOL bIncludeClockTime );

//------------------------------------------------------------------

extern void		DoSetMouseOverCursor( BOOL );

extern void		MakeSubDirectory( const char* pcFilename, const char* pcRootPath );
extern void		SysMakeSubDirectories( const char* pcFilename, const char* pcRootPath );


// 'quicker' (?) versions of stricmp etc that fail as soon as they know the strings dont match
// (i.e. they dont behave the same as stricmp etc so are not fully interchangeable)
extern int		tinstricmp( const char*, const char* );
extern int		tinstrcmp( const char*, const char* );
extern unsigned long long		tinstrtou64( const char* szVal, char** delimiters, int nBase );

// --------------------------- File resource interface

typedef struct
{
	int		nBytesTransferred;
	int		nBytesInFile;
	
} FetchFileProgress;

typedef enum
{
	FETCHFILE_INIT_ERROR = 0,
	FETCHFILE_SUCCESS = 1,
	FETCHFILE_NOT_FOUND = 2,
	FETCHFILE_FAILED = 3,

} FETCHFILE_RETURN_VAL;


typedef struct
{
	float	r;
	float	g;
	float	b;
	float	a;
} COLOUR;

typedef	void(*fnFetchFileCallback)( FETCHFILE_RETURN_VAL, long lParam, const char* szLocalFilename );


extern BOOL		FetchFileGetProgress( const char* szFilename, FetchFileProgress* pOut );

extern int		ArchiveMount( const char* szArchiveLocalFilename );
extern void		ArchiveRelease( int nArchiveHandle );
extern void		ArchiveExtract( int nArchiveHandle, const char* szDestinationPath );

extern uint32		GetColWithModifiedAlpha( uint32 ulCol, float fAlpha );
extern uint32		GetColValue( COLOUR xCol );
extern COLOUR		GetColFloats( uint32 ulCol );
extern uint32		GetBlendedCol( uint32 ulCol1, uint32 ulCol2, float fCol1BlendStrength );
#define	frand()		FRand()

extern float	FClamp( float fVal, float fMin, float fMax );
extern float	FSlerp( float fValFrom, float fValTo, float fPhase );
extern float	FRand( float, float );
extern int		RandInt( int, int );
extern int		ClampInt( int nVal, int nMin, int nMax );



#ifdef __cplusplus
}
#endif

/**********************************************
 **                                          **
 ** You are welcome to distribute, use,      **
 ** abuse, tinker with, criticise, point &   **
 ** laugh at anything to do with this        **
 ** library. It'd be nice if the URL         **
 ** was left around somewhere tho :)         **
 **                                          **
 ** Feedback, comments, etc etc to..         **
 **          mit2@gameislands.net            **
 **  or on the forums at..				     **
 **											 **
 **    http://theuniversal.net				 **
 **                                          **
 *********************************************/
#endif // #ifndef UNIVERSAL_CODEUTIL_H

