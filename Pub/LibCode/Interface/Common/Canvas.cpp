
#include "StandardDef.h"
#include "Interface.h"

const char*		CanvasTextGetTag( const char* pcRunner, char* pcTagOut )
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

void	CanvasTextGetTagKeyValue( const char* acTagBuff, char* acKey, char* acVal )
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

void		CanvasText( int nLayer, float fX, float fY, const char* szText, uint32 ulCol, int nFont, float fMaxWidth, float fScale )
{
const char*		pcRunner;
char			acTextBuff[1024];
const char*		pcStringStart = szText;
char			acTagBuff[128];
int				nOriginalFlags = 0;//mnCurrentFontFlags;
uint32			ulOriginalCol = ulCol;
int				nNewFlags = 0;//mnCurrentFontFlags;
int				nCurrentFlags = 0;
int				nSegmentLen = 0;
const char*		pcTagStart;
int				nSegmentX;
int				nY;
uint32			ulNewCol = ulCol;
int				nSegmentMaxW;
int				nStringWidth;
char*			pcSecondLine;

	nSegmentX = (int)( InterfaceGetWidth() * fX );
	nY = (int)( InterfaceGetHeight() * fY );
	nSegmentMaxW = (int)( InterfaceGetWidth() * fMaxWidth );

	// Parse line for any tags..
	pcRunner = szText;
	while( *pcRunner != 0 )
	{
		if ( *pcRunner == '<' )
		{
			pcTagStart = pcRunner;
			pcRunner = CanvasTextGetTag( pcRunner, acTagBuff );

			// TODO - Parse tag...
			if ( acTagBuff[0] != 0 )
			{
			char	acKey[64];
			char	acVal[64];

				CanvasTextGetTagKeyValue( acTagBuff, acKey, acVal );

				if ( stricmp( acKey, "b" ) == 0 )
				{
					nNewFlags = nCurrentFlags | FONT_FLAG_BOLD;
				}
				else if ( stricmp( acKey, "/b" ) == 0 )
				{
					nNewFlags = nCurrentFlags & ~FONT_FLAG_BOLD;
				}
				else if ( stricmp( acKey, "i" ) == 0 )
				{
					nNewFlags = nCurrentFlags | FONT_FLAG_ITALIC;
				}
				else if ( stricmp( acKey, "/i" ) == 0 )
				{
					nNewFlags = nCurrentFlags & ~FONT_FLAG_ITALIC;
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
					InterfaceSetFontFlags( nCurrentFlags );
					if ( nSegmentMaxW <= 0 )
					{
						InterfaceText( nLayer, nSegmentX, nY, acTextBuff, ulCol, nFont );
					}
					else
					{
						InterfaceTextLimitWidth( nLayer, nSegmentX, nY, acTextBuff, ulCol, nFont, nSegmentMaxW );
					}
				}

				nStringWidth = GetStringWidth( acTextBuff, nFont );
				nSegmentX += nStringWidth;
				nSegmentMaxW -= nStringWidth;
				nCurrentFlags = nNewFlags;
				ulCol = ulNewCol;
				pcStringStart = pcRunner;
			}
		}
		else
		{
			pcRunner++;
		}
	}
		
	InterfaceSetFontFlags( nCurrentFlags );
	if ( nSegmentMaxW > 0 )
	{
		pcSecondLine = InterfaceTextLimitWidth( nLayer, nSegmentX, nY, pcStringStart, ulCol, nFont, nSegmentMaxW );
		if ( ( pcSecondLine ) &&
			 ( pcSecondLine[0] != 0 ) )
		{
			nY += 18;
			nSegmentX = (int)( InterfaceGetWidth() * fX );
			nSegmentMaxW = (int)( InterfaceGetWidth() * fMaxWidth );
			InterfaceTextLimitWidth( nLayer, nSegmentX, nY, pcSecondLine, ulCol, nFont, nSegmentMaxW );
		}
	}
	else
	{
		InterfaceText( nLayer, nSegmentX, nY, pcStringStart, ulCol, nFont );
	}

	InterfaceSetFontFlags( 0 );

}
