
#include <StandardDef.h>
#include <Interface.h>

#include "StringList.h"

#define			DEFAULT_SIZE_OF_DISPLAY_STRING_BUFFER		65536

char*			mpcStringBufferMemory = NULL;
char*			mpcStringBufferRunner = NULL;
char*			mpcStringBufferEnd = NULL;
int				mnStringBufferSize = 0;

TEXT_BUFFER			maxTextBuffer[ MAX_STRINGS_IN_BUFFER ];
int		mnPosInTextBuffer = 0;
int		mnCurrentFontFlags = 0;

char*	InterfaceGetStringBufferMem( const char* szString )
{
int			nStringLen = strlen( szString );
char*		pcMem = mpcStringBufferRunner;

	if ( mpcStringBufferRunner + nStringLen + 1 >= mpcStringBufferEnd )
	{
		// todo - realloc
	}
	memcpy( pcMem, szString, nStringLen + 1 );
	mpcStringBufferRunner += nStringLen + 1;
	return( pcMem );
}

INTERFACE_API void	InterfaceSetFontFlags( int nFlag)
{
	mnCurrentFontFlags = nFlag;
}


BOOL	ShouldAddString( const char* szString )
{
int		nLen;
	if ( !szString ) return( FALSE );

	nLen = strlen( szString );
	if ( nLen == 0 )
	{
		return( FALSE );
	}
	// Dont draw strings that are just a space
	else if ( ( nLen == 1 ) &&	
			  ( szString[0] == ' ' ) )
	{
		return( FALSE );
	}
	return( TRUE );
}


INTERFACE_API void	InterfaceTextScaled( int nLayer, int nX, int nY, const char* szString, ulong ulCol, int font, float fSize )
{
TEXT_BUFFER*	pcTextBuffer = maxTextBuffer + mnPosInTextBuffer;

	if ( ShouldAddString( szString ) == FALSE )
	{
		return;
	}

	pcTextBuffer->pcString = InterfaceGetStringBufferMem( szString );

	pcTextBuffer->nX = nX;// + mnInterfaceDrawX;
	pcTextBuffer->nY = nY;// + mnInterfaceDrawY;

	pcTextBuffer->ulCol = ulCol;
	pcTextBuffer->wAlign = ALIGN_SCALED;
	pcTextBuffer->wLayer = (short)nLayer;
	pcTextBuffer->wFlag = (short)(mnCurrentFontFlags);
	pcTextBuffer->bFont = (byte)(font);
	pcTextBuffer->fScale = fSize;

	mnPosInTextBuffer++;

	if ( mnPosInTextBuffer == MAX_STRINGS_IN_BUFFER )
	{
		mnPosInTextBuffer = 1;
//		PANIC_IF( FALSE, "Too many strings added to text buffer" )
	}

}


const char*		InterfaceTextGetTag( const char* pcRunner, char* pcTagOut )
{
int		nCount = 0;
const char*		pcRunnerStart = pcRunner;
char*		pcTagOutRunner = pcTagOut;

	*pcTagOut = 0;
	
	// Skip past the '<'
	pcRunner++;

	// not a valid tag
	if ( ( *pcRunner == ' ' ) ||
		 ( *pcRunner == 0 ) )
	{
		return( pcRunnerStart + 1 );
	}
	else
	{
		while( *pcRunner != '>' )
		{
			*pcTagOutRunner = *pcRunner;
			pcTagOutRunner++;
			// reached end without tag end..
			if ( *pcRunner == 0 )
			{
				*pcTagOut = 0;
				return( pcRunnerStart + 1 );
			}

			nCount++;
			if ( nCount > 64 )
			{
				*pcTagOut = 0;
				return( pcRunnerStart + 1 );
			}
			pcRunner++;
		}

		*pcTagOutRunner = 0;
		return( pcRunner + 1 );
	}
}

void	InterfaceTextGetTagKeyValue( const char* acTagBuff, char* acKey, char* acVal )
{
const char*		pcRunner = acTagBuff;
char*			pcOutRunner = acKey;

	while( *pcRunner != 0 )
	{
		if ( *pcRunner == '=' )
		{
			*pcOutRunner = 0;
			pcOutRunner = acVal;
			pcRunner++;
		}
		else
		{
			*pcOutRunner++ = *pcRunner++;
		}
	}
	*pcOutRunner = 0;
}


/***************************************************************************
 * Function    : InterfaceTextRight
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void InterfaceTextRight( int nLayer, int nX, int nY, const char* szString, ulong ulCol, int nFont )
{
TEXT_BUFFER*	pcTextBuffer = maxTextBuffer + mnPosInTextBuffer;

	if ( ShouldAddString( szString ) == FALSE )
	{
		return;
	}

	pcTextBuffer->pcString = InterfaceGetStringBufferMem( szString );

	pcTextBuffer->nX = nX;// + mnInterfaceDrawX;
	pcTextBuffer->nY = nY;// + mnInterfaceDrawY;

	pcTextBuffer->ulCol = ulCol;
	pcTextBuffer->wAlign = ALIGN_RIGHT;
	pcTextBuffer->wLayer = (short)nLayer;
	pcTextBuffer->wFlag = (short)(mnCurrentFontFlags);
	pcTextBuffer->bFont = (byte)(nFont);

	mnPosInTextBuffer++;

	if ( mnPosInTextBuffer == MAX_STRINGS_IN_BUFFER )
	{
		mnPosInTextBuffer = 1;
//		PANIC_IF( FALSE, "Too many strings added to text buffer" )
	}
}

INTERFACE_API int	InterfaceTextRect( int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth, int nMaxHeight )
{
TEXT_BUFFER*	pxTextBuffer = maxTextBuffer + mnPosInTextBuffer;

	if ( ShouldAddString( szString ) == FALSE )
	{
		return( 0 );
	}

	pxTextBuffer->pcString = InterfaceGetStringBufferMem( szString );

	pxTextBuffer->nX = nX;// + mnInterfaceDrawX;
	pxTextBuffer->nY = nY;// + mnInterfaceDrawY;
	pxTextBuffer->nWidth = nMaxWidth;
	pxTextBuffer->nHeight = nMaxHeight;

	pxTextBuffer->ulCol = ulCol;
	pxTextBuffer->wAlign = ALIGN_RECT;
	pxTextBuffer->wLayer = (short)nLayer;
	pxTextBuffer->wFlag = (short)(mnCurrentFontFlags);
	pxTextBuffer->bFont = (byte)(font);

	mnPosInTextBuffer++;

	if ( mnPosInTextBuffer == MAX_STRINGS_IN_BUFFER )
	{
		mnPosInTextBuffer = 1;
//		PANIC_IF( FALSE, "Too many strings added to text buffer" )
	}
	// todo - return number of lines occupied
	return( InterfaceTextGetHeightUsed( szString, font, nMaxWidth ) );
}

INTERFACE_API int	InterfaceTextBox( int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth, BOOL bLeftAlign )
{
TEXT_BUFFER*	pxTextBuffer = maxTextBuffer + mnPosInTextBuffer;

	if ( ShouldAddString( szString ) == FALSE )
	{
		return( 0 );
	}

	pxTextBuffer->pcString = InterfaceGetStringBufferMem( szString );
	strcpy( pxTextBuffer->pcString, szString );

	pxTextBuffer->nX = nX;// + mnInterfaceDrawX;
	pxTextBuffer->nY = nY;// + mnInterfaceDrawY;
	pxTextBuffer->nWidth = nMaxWidth;

	pxTextBuffer->ulCol = ulCol;
	if ( bLeftAlign )
	{
		pxTextBuffer->wAlign = ALIGN_BOX;
	}
	else
	{
		pxTextBuffer->wAlign = ALIGN_BOX_CENTRE;
	}
	pxTextBuffer->wLayer = (short)nLayer;
	pxTextBuffer->wFlag = (short)(mnCurrentFontFlags);
	pxTextBuffer->bFont = (byte)(font);

	mnPosInTextBuffer++;

	if ( mnPosInTextBuffer == MAX_STRINGS_IN_BUFFER )
	{
		mnPosInTextBuffer = 1;
//		PANIC_IF( FALSE, "Too many strings added to text buffer" )
	}
	// todo - return number of lines occupied
	return( InterfaceTextGetHeightUsed( szString, font, nMaxWidth ) );
}

INTERFACE_API char*	InterfaceTextLimitWidth( int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth )
{
	InterfaceTextBox( nLayer, nX, nY, szString, ulCol, font, nMaxWidth, TRUE );
	// TODO - Is supposed to return the point in the string we were able to render up to..
	return( NULL );
}

INTERFACE_API void	InterfaceTextCenter( int nLayer, int nX1, int nX2, int nY, const char* szString, ulong ulCol, int font )
{
TEXT_BUFFER*	pxTextBuffer = maxTextBuffer + mnPosInTextBuffer;

	if ( ShouldAddString( szString ) == FALSE )
	{
		return;
	}

	pxTextBuffer->pcString = InterfaceGetStringBufferMem( szString );
	strcpy( pxTextBuffer->pcString, szString );

	pxTextBuffer->nX = nX1;// + mnInterfaceDrawX;
	pxTextBuffer->nY = nY;// + mnInterfaceDrawY;
	pxTextBuffer->nWidth = nX2 - nX1;

	pxTextBuffer->ulCol = ulCol;
	pxTextBuffer->wAlign = ALIGN_CENTER;
	pxTextBuffer->wLayer = (short)nLayer;
	pxTextBuffer->wFlag = (short)(mnCurrentFontFlags);
	pxTextBuffer->bFont = (byte)(font);

	mnPosInTextBuffer++;

	if ( mnPosInTextBuffer == MAX_STRINGS_IN_BUFFER )
	{
		mnPosInTextBuffer = 1;
//		PANIC_IF( FALSE, "Too many strings added to text buffer" )
	}
}


INTERFACE_API void	InterfaceTextCentre( int nLayer, int nX, int nY, const char* szString, ulong ulCol, int font )
{
	InterfaceTextCenter( nLayer, nX - 600, nX + 600, nY, szString, ulCol, font );
}



void		InterfaceTextAddString( int nLayer, int nX, int nY, const char* szString, ulong ulCol, int nFont, int nFontFlags )
{
TEXT_BUFFER*	pxTextBuffer = maxTextBuffer + mnPosInTextBuffer;

	if ( ShouldAddString( szString ) == FALSE )
	{
		return;
	}
	pxTextBuffer->pcString = InterfaceGetStringBufferMem( szString );
	
	pxTextBuffer->nX = nX;// + mnInterfaceDrawX;
	pxTextBuffer->nY = nY;// + mnInterfaceDrawY;

	pxTextBuffer->ulCol = ulCol;
	pxTextBuffer->wAlign = ALIGN_LEFT;
	pxTextBuffer->wLayer = (short)nLayer;
	pxTextBuffer->wFlag = (short)(nFontFlags);
	pxTextBuffer->bFont = (byte)(nFont);

	mnPosInTextBuffer++;

	if ( mnPosInTextBuffer == MAX_STRINGS_IN_BUFFER )
	{
		mnPosInTextBuffer = 1;
//		PANIC_IF( FALSE, "Too many strings added to text buffer" )
	}

}

/***************************************************************************
 * Function    : InterfaceText
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void InterfaceText( int nLayer, int nX, int nY, const char* szString, ulong ulCol, int nFont )
{
const char*		pcRunner;
char			acTextBuff[1024];
const char*		pcStringStart = szString;
char			acTagBuff[128];
int				nOriginalFlags = mnCurrentFontFlags;
ulong			ulOriginalCol = ulCol;
int				nNewFlags = mnCurrentFontFlags;
int				nSegmentLen = 0;
const char*		pcTagStart;
int				nSegmentX = nX;
ulong			ulNewCol = ulCol;

	// Parse line for any tags..
	pcRunner = szString;
	while( *pcRunner != 0 )
	{
		if ( *pcRunner == '<' )
		{
			pcTagStart = pcRunner;
			pcRunner = InterfaceTextGetTag( pcRunner, acTagBuff );

			// TODO - Parse tag...
			if ( acTagBuff[0] != 0 )
			{
			char	acKey[64];
			char	acVal[64];

				InterfaceTextGetTagKeyValue( acTagBuff, acKey, acVal );

				if ( stricmp( acKey, "b" ) == 0 )
				{
					nNewFlags = mnCurrentFontFlags | FONT_FLAG_BOLD;
				}
				else if ( stricmp( acKey, "/b" ) == 0 )
				{
					nNewFlags = mnCurrentFontFlags & ~FONT_FLAG_BOLD;
				}
				else if ( stricmp( acKey, "i" ) == 0 )
				{
					nNewFlags = mnCurrentFontFlags | FONT_FLAG_ITALIC;
				}
				else if ( stricmp( acKey, "/i" ) == 0 )
				{
					nNewFlags = mnCurrentFontFlags & ~FONT_FLAG_ITALIC;
				}
				else if ( stricmp( acKey, "col" ) == 0 )
				{
					ulNewCol = strtoul( acVal, NULL, 16 );
					if ( (ulNewCol & 0xff000000) == 0 )
					{
						ulNewCol |= 0xF0000000;
					}
				}
				else if ( stricmp( acKey, "/col" ) == 0 )
				{
					ulNewCol = ulOriginalCol;
				}

				nSegmentLen = (int)( pcTagStart - pcStringStart );
				memcpy( acTextBuff, pcStringStart, nSegmentLen + 1 );
				acTextBuff[nSegmentLen] = 0;
				if ( nSegmentLen > 0 )
				{
					InterfaceTextAddString( nLayer, nSegmentX, nY, acTextBuff, ulCol, nFont, mnCurrentFontFlags );
				}

				nSegmentX += GetStringWidth( acTextBuff, nFont );
				mnCurrentFontFlags = nNewFlags;
				ulCol = ulNewCol;
				pcStringStart = pcRunner;
			}
		}
		else
		{
			pcRunner++;
		}
	}

	InterfaceTextAddString( nLayer, nSegmentX, nY, pcStringStart, ulCol, nFont, mnCurrentFontFlags );

}


TEXT_BUFFER*		StringListGetNext( int nLayer, TEXT_BUFFER* pLast )
{
TEXT_BUFFER*	pxTextBuffer;
int			nPos;

	if ( pLast ) 
	{
		pxTextBuffer = pLast + 1;
	}
	else 
	{
		pxTextBuffer = maxTextBuffer;
	}
	nPos = (int)( pxTextBuffer - maxTextBuffer );

	while( ( nPos < mnPosInTextBuffer ) &&
		   ( pxTextBuffer->wLayer != nLayer ) )
	{
		pxTextBuffer++;
		nPos++;
	}

	if ( pxTextBuffer->pcString )
	{
		return( pxTextBuffer );
	}
	return( NULL );

}

void		StringListInit( void )
{
	memset( maxTextBuffer, 0, sizeof( TEXT_BUFFER ) * MAX_STRINGS_IN_BUFFER );
	mnPosInTextBuffer = 0;

	mpcStringBufferMemory = (char*)malloc( DEFAULT_SIZE_OF_DISPLAY_STRING_BUFFER );
	mpcStringBufferRunner = mpcStringBufferMemory;
	mnStringBufferSize = DEFAULT_SIZE_OF_DISPLAY_STRING_BUFFER;
	mpcStringBufferEnd = mpcStringBufferMemory + mnStringBufferSize;
}


void	StringListReset( void )
{
	mnPosInTextBuffer = 0;
	mpcStringBufferRunner = mpcStringBufferMemory;
}

void	StringListShutdown( void )
{
	free( mpcStringBufferMemory );
	mpcStringBufferMemory = NULL;
	mpcStringBufferRunner = NULL;
	mnStringBufferSize = 0;
	mpcStringBufferEnd = NULL;

}