#ifdef TUD9
#include "../../DirectX/InterfaceInternalsDX.h"
#endif


#include <stdio.h>

#include <StandardDef.h>
#include <Interface.h>

#include "../Overlays/Overlays.h"
#include "../InterfaceCommon.h"
#include "FontCommon.h"
#ifdef TUD9
#include "../../DirectX/FontDX.h"
#endif
#ifdef USING_OPENGL
#include "../../OpenGL/FontGL.h"
#endif
#include "../../../../../GameCommon/Util/cJSON.h"

TEXT_BUFFER			maxTextBuffer[ MAX_STRINGS_IN_BUFFER ];
int		mnPosInTextBuffer = 0;

CFontDef*	mpFontDefs[MAX_FONTS_IN_GAME] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

FLATVERTEX*			mpFontVertices = NULL;
int					mnFontVertexIndex = 0;
int					mnCurrentFontFlags = 0;
float	mfGiantFontScale = 2.0f;

uint32		manFontColours[] =
{
	0xE0F0F0F0,			/* UNUSED */
	0xE0F0F0F0,			/* COL_SYS_SPECIAL */
	0xA080F080,			/* COL_PLAYER_ANNOUNCE */
	0xE0F0A838,			/* COL_TEAM */
	0xD0A0B0F0,			/* COL_PUBLIC */
	0xE0B0F090,			/* COL_PRIVATE  5 */
	0xF0D070E0,			/* COL_SYSOP  */
	0x80B0B0B0,			/* COL_SERVER_MESSAGE */
	0xE09080F8,			/* COL_SERVER_MESSAGE_2 */
	0xFFF05030,			/* COL_WARNING  */
	0xC0F0E090,			/* COL_PLAYER_LABEL (NOT FOR CONSOLE USE :] ) */
	0xD0E0F0F0,			/* COL_PLAYER_LIST */
	0xD0000000,			/* COL_FONT_BLACK (For newspaper) */
	0xD0F0F080,			/* COL_COMMS_SPECIAL */
	0xB0C0C0C0,			/* COL_SYS_SPECIAL_FADED */
	0xC0E0E0A0,			/* COL_COMMS_SPECIAL_TRANS 15 */
	0xB0B0C0C0,			/* COL_PLAYER_LIST_FADED */
	0xB0B090A0,			/* COL_DISABLED */
	0xE03060B0,			/* COL_TRANSACTION */
	0xF0F0F0F8,			/* COL_WHITE */
	0xD090C0A0,			/* COL_SYSMESSAGE_GOOD 20 */
	0xF0F06040,		 	/* COL_CHAT */
	0xF0F03060,			/* COL_CHAT2 */ 
	0xC0A09010,			/* COL_BARTER */
	0xF098C0F0,			/* COL_PUBLIC_DROP_SHADOW*/
	0xF0E0E0B0,			/* COL_CHATGROUP_DROP_SHADOW */
	0xF0F06040,			/* COL_SYSMSG_DROP_SHADOW */
	0xC0000000,			/* COL_FONT_2 */
	0xC0F0F080,			/* COL_F2_COMMS_SPECIAL */
	0xC0F0F0F0,			/* COL_F2_SYS_SPECIAL */
	0xF0F0F0F8,			/* COL_F2_WHITE */
	0xC0A0B0F0,			/* COL_F2_PUBLIC */
	0xC0000000,			/* COL_F2_BLACK (For newspaper) */
	0xF000e0f0,			/* COL_F2_CARDSRED */
	0xD0A0B0F0,			/* COL_F2_SPARE2 */
	0xD0A0B0F0,			/* COL_F2_SPARE3 */
	0xD0A0B0F0,			/* COL_F2_SPARE4 */
	0xD0A0B0F0,			/* COL_F2_SPARE5 */
	0xD0A0B0F0,			/* COL_F2_SPARE6 */
	0xD0000000,			/* COL_FONT_3 */
	0xD0F0F080,			/* COL_F3_COMMS_SPECIAL */
	0xD0F0F0F0,			/* COL_F3_SYS_SPECIAL */
	0xD0F0F0F8,			/* COL_F3_WHITE */
	0xD0A0B0F0,			/* COL_F3_PUBLIC */
	0xD0000000,			/* COL_F3_BLACK (For newspaper) */
	0xE0F0A838,			/* COL_F3_TEAM */
	0xB0A0B0F0,			/* COL_F3_INFO1 */
	0xB0109010,			/* COL_F3_INFO2 */
	0xB0A0B0F0,			/* COL_F3_INFO3 */
	0xE8F04020,			/* COL_F3_WARNING */
	0xB0A0B0F0,			/* COL_F3_SPARE2 */
	0xB0A0B0F0,			/* COL_F3_SPARE3 */
	0xB0000000,			/* COL_FONT_4 */
	0xB0F0F080,			/* COL_F4_COMMS_SPECIAL */
	0xB0F0F0F0,			/* COL_F4_SYS_SPECIAL */
	0xF0F0F0F8,			/* COL_F4_WHITE */
	0xB0A0B0F0,			/* COL_F4_PUBLIC */
	0xD0000000,			/* COL_F4_BLACK (For newspaper) */
	0xE0F0A838,			/* COL_F4_TEAM */
	0xB06070F0,			/* COL_F4_INFO1 */
	0xB0A0B0F0,			/* COL_F4_INFO2 */
	0xB0A0B0F0,			/* COL_F4_INFO3 */
	0xE8F04020,			/* COL_F4_WARNING */
};



/***************************************************************************
 * Function    : AddCharVertices
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void AddCharVertices( FLOATRECT* pxScrRect, FLOATRECT* pxTexRect, uint32 ulCol, int nFlag )
{
float	fItalicOffset = 0.0f;

	if ( nFlag & FONT_FLAG_ITALIC )
	{
		fItalicOffset = 3.0f;
	}

	if ( mnFontVertexIndex > (SIZE_OF_FONT_VERTEX_BUFFER - 12 ) )
	{
#ifdef TUD9
		DrawFontBufferDX();
#endif
#ifdef USING_OPENGL
		DrawFontBufferGL();
#endif
	}

	if ( mpFontVertices == NULL )
	{
		return;
	}

	mpFontVertices[ mnFontVertexIndex ].x = pxScrRect->x1 + fItalicOffset;
	mpFontVertices[ mnFontVertexIndex ].y = pxScrRect->y1;
	mpFontVertices[ mnFontVertexIndex ].z = 1.0f;
//	mpFontVertices[ mnFontVertexIndex ].rhw = 1.0f;
	mpFontVertices[ mnFontVertexIndex ].color    = ulCol;
	mpFontVertices[ mnFontVertexIndex ].tu       = pxTexRect->x1;
	mpFontVertices[ mnFontVertexIndex ].tv       = pxTexRect->y1;
	mnFontVertexIndex++;

	mpFontVertices[ mnFontVertexIndex ].x = pxScrRect->x2 + fItalicOffset;
	mpFontVertices[ mnFontVertexIndex ].y = pxScrRect->y1;
	mpFontVertices[ mnFontVertexIndex ].z = 1.0f;
//	mpFontVertices[ mnFontVertexIndex ].rhw = 1.0f;
	mpFontVertices[ mnFontVertexIndex ].color    = ulCol;
	mpFontVertices[ mnFontVertexIndex ].tu       = pxTexRect->x2;
	mpFontVertices[ mnFontVertexIndex ].tv       = pxTexRect->y1;
	mnFontVertexIndex++;


	mpFontVertices[ mnFontVertexIndex ].x = pxScrRect->x2;
	mpFontVertices[ mnFontVertexIndex ].y = pxScrRect->y2;
	mpFontVertices[ mnFontVertexIndex ].z = 1.0f;
//	mpFontVertices[ mnFontVertexIndex ].rhw = 1.0f;
	mpFontVertices[ mnFontVertexIndex ].color    = ulCol;
	mpFontVertices[ mnFontVertexIndex ].tu       = pxTexRect->x2;
	mpFontVertices[ mnFontVertexIndex ].tv       = pxTexRect->y2;
	mnFontVertexIndex++;


	mpFontVertices[ mnFontVertexIndex ].x = pxScrRect->x1;
	mpFontVertices[ mnFontVertexIndex ].y = pxScrRect->y2;
	mpFontVertices[ mnFontVertexIndex ].z = 1.0f;
//	mpFontVertices[ mnFontVertexIndex ].rhw = 1.0f;
	mpFontVertices[ mnFontVertexIndex ].color    = ulCol;
	mpFontVertices[ mnFontVertexIndex ].tu       = pxTexRect->x1;
	mpFontVertices[ mnFontVertexIndex ].tv       = pxTexRect->y2;
	mnFontVertexIndex++;

	mpFontVertices[ mnFontVertexIndex ].x = pxScrRect->x1 + fItalicOffset;
	mpFontVertices[ mnFontVertexIndex ].y = pxScrRect->y1;
	mpFontVertices[ mnFontVertexIndex ].z = 1.0f;
//	mpFontVertices[ mnFontVertexIndex ].rhw = 1.0f;
	mpFontVertices[ mnFontVertexIndex ].color    = ulCol;
	mpFontVertices[ mnFontVertexIndex ].tu       = pxTexRect->x1;
	mpFontVertices[ mnFontVertexIndex ].tv       = pxTexRect->y1;
	mnFontVertexIndex++;

	mpFontVertices[ mnFontVertexIndex ].x = pxScrRect->x2;
	mpFontVertices[ mnFontVertexIndex ].y = pxScrRect->y2;
	mpFontVertices[ mnFontVertexIndex ].z = 1.0f;
//	mpFontVertices[ mnFontVertexIndex ].rhw = 1.0f;
	mpFontVertices[ mnFontVertexIndex ].color    = ulCol;
	mpFontVertices[ mnFontVertexIndex ].tu       = pxTexRect->x2;
	mpFontVertices[ mnFontVertexIndex ].tv       = pxTexRect->y2;
	mnFontVertexIndex++;

}


/***************************************************************************
 * Function    : InterfaceGetARGBForColour
 * Params      : 
 * Returns     : hex colour number(i hope)
 * Description : added by Kim, to get a hex colour number.
 **************************************************************************/
uint32 InterfaceGetARGBForColour(uint32 nCol)
{
	// If its less than 0x1ff its a colour code
	if ( ( nCol < 0x1FF ) &&
		 ( nCol >= 0 ) )
	{
		return( manFontColours[ nCol ] );
	}
	// Otherwise its already in ARGB format
	return( nCol );
}







void	CFontDef::Initialise( const char* szDefName)
{
int		nStrlen = strlen( szDefName );

	if ( stricmp( szDefName + (nStrlen-4), "json") == 0 )
	{
		m_bIsBottomAligned = TRUE;
		// JSON Format for font texture from:   https://evanw.github.io/font-texture-generator/
		ReadJSONLayoutFile( szDefName );
	}
	else
	{
		ReadFontrastFile( szDefName );
	}
}

void	CFontDef::ReadJSONLayoutFile( const char* szDefName )
{
FILE*	pFile;
cJSON*	pJSONRoot;
cJSON*	pCharactersArray;
cJSON*	pCharItem;
cJSON*	pCharDetails;
byte*	pbMem;
int		nNumCharacters;
int		nLoop;
int		nFileSize;
float	fTextureSizeY;
float	fTextureSizeX;
int		nCharNum;

	ZeroMemory( maFontLookup, 256 * sizeof(FONT_UVCHAR) );
	
	pFile = fopen( szDefName, "rb" );
	if ( pFile )
	{
		nFileSize = SysGetFileSize( pFile );

		pbMem = (byte*)malloc(	nFileSize + 1 );
		pbMem[nFileSize] = 0;
		fread( pbMem, nFileSize, 1, pFile );
		fclose( pFile );
	
		pJSONRoot = cJSON_Parse( (const char*)pbMem );

		fTextureSizeX = (float)cJSON_GetObjectValueInt( pJSONRoot, "width");
		fTextureSizeY = (float)cJSON_GetObjectValueInt( pJSONRoot, "height");
		
		mnFontOverallSize = cJSON_GetObjectValueInt( pJSONRoot, "size");

		pCharactersArray = cJSON_GetObjectItem( pJSONRoot, "characters" );
		nNumCharacters = cJSON_GetArraySize( pCharactersArray );

		for( nLoop = 0; nLoop < nNumCharacters; nLoop++ )
		{
			pCharItem = cJSON_GetArrayItem( pCharactersArray, nLoop );

			nCharNum = (int)( *pCharItem->string );

			if ( ( nCharNum >= 0 ) &&
				 ( nCharNum < 256 ) )
			{
				pCharDetails = cJSON_GetObjectItem( pCharItem, pCharItem->string );

				maFontLookup[ nCharNum ].u = ( cJSON_GetObjectValueInt( pCharItem, "x" ) / fTextureSizeX );
				maFontLookup[ nCharNum ].v = ( cJSON_GetObjectValueInt( pCharItem, "y") / fTextureSizeY );
				maFontLookup[ nCharNum ].w = ( cJSON_GetObjectValueInt( pCharItem, "width") / fTextureSizeX );
				maFontLookup[ nCharNum ].h = ( cJSON_GetObjectValueInt( pCharItem, "height") / fTextureSizeY );

				maFontLookup[ nCharNum ].originX = ( cJSON_GetObjectValueInt( pCharItem, "originX") / fTextureSizeX );
				maFontLookup[ nCharNum ].originY = ( cJSON_GetObjectValueInt( pCharItem, "originY") / fTextureSizeY );
				maFontLookup[ nCharNum ].advance = ( cJSON_GetObjectValueInt( pCharItem, "advance") / fTextureSizeX );
			}
		}

		cJSON_Delete( pJSONRoot );
		free( pbMem );
	}
}


void	CFontDef::ReadFontrastFile( const char* szDefName )
{
FILE*	pFile;
byte*	pbMem;
int		nFileSize;
int		nLoop = 0;
int		nBaseLine;
int		anVals[16];
int		nNextVal = 0;
int		nCharNum;
float	fTextureSizeY;
float	fTextureSizeX;

	if ( m_TextureSizeY == 0 ) m_TextureSizeY = 256;
	if ( m_TextureSizeX == 0 ) m_TextureSizeX = 256;

	// TEMP - width of font textures is always assumed to be 256.0f 
	// (we cant rely on the loaded texture size as some cards seem to auto-resize em)
	fTextureSizeY = float( m_TextureSizeY );
	fTextureSizeX = 256.0f;//float( m_TextureSizeX );

	ZeroMemory( maFontLookup, 256 * sizeof(FONT_UVCHAR) );
	
	pFile = fopen( szDefName, "rb" );
	if ( pFile )
	{
		nFileSize = SysGetFileSize( pFile );

		pbMem = (byte*)malloc(	nFileSize + 1 );
		pbMem[nFileSize] = 0;
		fread( pbMem, nFileSize, 1, pFile );
		fclose( pFile );

		nBaseLine = 0;
		while( pbMem[nLoop] != 0 )
		{
			while( ( pbMem[nLoop] != 0 ) &&
				   ( pbMem[nLoop] != '\t' ) &&
				   ( pbMem[nLoop] != '\r' ) &&
				   ( pbMem[nLoop] != '\n' ) )
			{
				nLoop++;
			}
			if ( pbMem[nLoop] == '\t' )
			{
				pbMem[nLoop] = 0;
				anVals[nNextVal] = strtol( (char*)(pbMem + nBaseLine), NULL, 10 );
				nNextVal++;
				nLoop++;
				nBaseLine = nLoop;
			}
			else if ( ( pbMem[nLoop] == '\r' ) ||
				      ( pbMem[nLoop] == '\n' ) )
			{
				if ( nNextVal > 0 )
				{
					nCharNum = anVals[0];
					if ( ( nCharNum >= 0 ) && ( nCharNum < 256 ) )
					{
						maFontLookup[ nCharNum ].u = (float)( anVals[2] ) / fTextureSizeX;
						maFontLookup[ nCharNum ].v = (float)( anVals[3] ) / fTextureSizeY;
						maFontLookup[ nCharNum ].w = (float)( anVals[4] ) / fTextureSizeX;
						maFontLookup[ nCharNum ].h = (float)( anVals[5] ) / fTextureSizeY;
						PANIC_IF( anVals[5] > fTextureSizeY, "Height error in font load" );
					}
				}
				nNextVal = 0;
				nLoop++;
				nBaseLine = nLoop;
			}
		}
		free( pbMem );
	}
}



void	FontDefFreeAllGraphics( void )
{
int		nLoop;
	for ( nLoop = 0; nLoop < MAX_FONTS_IN_GAME; nLoop++ )
	{
		if ( mpFontDefs[nLoop] )
		{
			mpFontDefs[nLoop]->FreeTexture();
		}
	}
}

void	FontDefFreeAll( void )
{
int		nLoop;
	for ( nLoop = 0; nLoop < MAX_FONTS_IN_GAME; nLoop++ )
	{
		if ( mpFontDefs[nLoop] )
		{
			delete mpFontDefs[nLoop];
			mpFontDefs[nLoop] = NULL;
		}
	}
}


BOOL	InterfaceFontIsFixedWidth( int nFontNum )
{
	if ( ( nFontNum < MAX_FONTS_IN_GAME ) &&
		 ( mpFontDefs[nFontNum] ) )
	{
		return( mpFontDefs[nFontNum]->IsFixedWidth() );
	}
	return( FALSE );

}

BOOL	InterfaceFontIsFilteringOn( int nFontNum )
{
	if ( ( nFontNum < MAX_FONTS_IN_GAME ) &&
		 ( mpFontDefs[nFontNum] ) )
	{
		return( mpFontDefs[nFontNum]->IsFilteringOn() );
	}
	return( TRUE );

}

BOOL	InterfaceFontSetAsCurrentTexture( int nFontNum )
{
	if ( ( nFontNum < MAX_FONTS_IN_GAME ) &&
		 ( mpFontDefs[nFontNum] ) )
	{
		mpFontDefs[nFontNum]->SetTextureAsCurrent();
		return( TRUE );
	}
	return( FALSE );
}

BOOL	InterfaceFontLookupChar( int nFontNum, char cChar, FONT_UVCHAR* pOut )
{
	if ( ( nFontNum < MAX_FONTS_IN_GAME ) &&
		 ( mpFontDefs[nFontNum] ) )
	{
		mpFontDefs[nFontNum]->LookupChar( cChar, pOut );
		return( TRUE );
	}
	return( FALSE );
}

BOOL	InterfaceFontSetFixedOffsets( int nFontNum, int nPosOffsetX, int nPosOffsetY, int nOccupyWidthReduction, int nOccupyHeightReduction )
{
	if ( nFontNum < MAX_FONTS_IN_GAME )
	{
		if ( mpFontDefs[nFontNum] )
		{
			mpFontDefs[nFontNum]->mnFontOccupyWidthReduction = nOccupyWidthReduction;
			mpFontDefs[nFontNum]->mnFontOccupyHeightReduction = nOccupyHeightReduction;
			mpFontDefs[nFontNum]->mnFontDrawOffsetX = nPosOffsetX;
			mpFontDefs[nFontNum]->mnFontDrawOffsetY = nPosOffsetY;
		}
	}
	return( TRUE );
}

BOOL	InterfaceFontLoad( int nFontNum, const char* pcImageFileName, const char* pcLayoutFile, uint32 ulFlags )
{
	if ( nFontNum < MAX_FONTS_IN_GAME )
	{
		if ( mpFontDefs[nFontNum] )
		{
			delete mpFontDefs[nFontNum];
			mpFontDefs[nFontNum] = NULL;
		}
		mpFontDefs[nFontNum] = new CFontDef;
		mpFontDefs[nFontNum]->SetTextureFileName( pcImageFileName );
		mpFontDefs[nFontNum]->LoadTexture();
		mpFontDefs[nFontNum]->Initialise( pcLayoutFile );
		if ( ulFlags & 1 )
		{
			mpFontDefs[nFontNum]->EnableFiltering( FALSE );
		}
		if ( ulFlags & 2 )
		{
			mpFontDefs[nFontNum]->SetFixedWidth( TRUE, 10 );
		}
		return( TRUE );
	}
	return( FALSE );
}

void	InterfaceFontFree( int nFontNum )
{
	if ( nFontNum < MAX_FONTS_IN_GAME )
	{
		if ( mpFontDefs[nFontNum] )
		{
			delete mpFontDefs[nFontNum];
			mpFontDefs[nFontNum] = NULL;
		}
	}
}

//------------------------------------------------------------------------------------------------




void InterfaceSetFontFlags( int nFlag )
{
	mnCurrentFontFlags = nFlag;
}




BOOL	ShouldAddString( const char* szString )
{
	if ( ( szString == NULL ) ||
		 ( szString[0] == 0 ) )
	{
		return( FALSE );
	}
	return( TRUE );
}

int		InterfaceGetFontNumFromColVal( uint32 ulCol, int nFont )
{
	if ( ulCol < 0x1FF )
	{
		if ( ulCol < COL_FONT2 )
		{
			return( 0 );
		}
		else if ( ulCol < COL_FONT3 )
		{
			return( 1 );
		}
		else if ( ulCol < COL_FONT4 )
		{
			return( 2 );
		}
		else
		{
			return( 3 );
		}
	}
	else
	{
		return( nFont );
	}
}

/***************************************************************************
 * Function    : AddFontString
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void AddFontString( int nX, int nY, const char* szString, uint32 ulCol, int nFlag )
{
char*	pcTextBuffer;
int		nLen;

	if ( InterfaceIsSmall() == TRUE )
	{
		return;
	}
	if ( ShouldAddString( szString ) == FALSE )
	{
		return;
	}

	pcTextBuffer = maxTextBuffer[ mnPosInTextBuffer ].acString;
	nLen = strlen( szString );
	if ( nLen > (MAX_CHARS_IN_STRING-1) )
	{
		nLen = MAX_CHARS_IN_STRING-1;
	}
	memcpy( pcTextBuffer, szString, nLen );
	pcTextBuffer[nLen] = 0;

	maxTextBuffer[ mnPosInTextBuffer ].nX = nX + mnInterfaceDrawX;
	maxTextBuffer[ mnPosInTextBuffer ].nY = nY + mnInterfaceDrawY;
	maxTextBuffer[ mnPosInTextBuffer ].nCol = ulCol;
	maxTextBuffer[ mnPosInTextBuffer ].nAlign = ALIGN_LEFT;
	maxTextBuffer[ mnPosInTextBuffer ].nLayer = 0;
	maxTextBuffer[ mnPosInTextBuffer ].wFlag = (short)nFlag;
	maxTextBuffer[ mnPosInTextBuffer ].bFont = InterfaceGetFontNumFromColVal( ulCol, 0 );
	mnPosInTextBuffer++;

	if ( mnPosInTextBuffer == MAX_STRINGS_IN_BUFFER )
	{
		mnPosInTextBuffer = 1;
//		PANIC_IF( FALSE, "Too many strings added to text buffer" );
	}

} 


/***************************************************************************
 * Function    : AddFontStringCenter
 * Params      :
 * Description : 
 ***************************************************************************/
void AddFontStringCenter( int nY, int nX1, int nX2, const char* szString, uint32 ulCol )
{
char*	pcTextBuffer;
RECT		xAlignRect;
int			nLen;

	if ( ( InterfaceIsSmall() == TRUE ) ||
		 ( ShouldAddString( szString ) == FALSE ) )
	{
		return;
	}

	xAlignRect.top = nY + mnInterfaceDrawY;
	xAlignRect.left = nX1 + mnInterfaceDrawX;
	xAlignRect.right = nX2 + mnInterfaceDrawX;

	pcTextBuffer = maxTextBuffer[ mnPosInTextBuffer ].acString;
	nLen = strlen( szString ) + 1;
	if ( nLen > (MAX_CHARS_IN_STRING-1) )
	{
		nLen = MAX_CHARS_IN_STRING-1;
	}
	memcpy( pcTextBuffer, szString, nLen );
	pcTextBuffer[MAX_CHARS_IN_STRING-1] = 0;

	maxTextBuffer[ mnPosInTextBuffer ].nX = 0;
	maxTextBuffer[ mnPosInTextBuffer ].nY = 0;

	maxTextBuffer[ mnPosInTextBuffer ].nCol = ulCol;
	maxTextBuffer[ mnPosInTextBuffer ].nAlign = ALIGN_CENTRE;
	maxTextBuffer[ mnPosInTextBuffer ].xAlignRect = xAlignRect;
	maxTextBuffer[ mnPosInTextBuffer ].nLayer = 0;
	maxTextBuffer[ mnPosInTextBuffer ].wFlag = (short)mnCurrentFontFlags;
	maxTextBuffer[ mnPosInTextBuffer ].bFont = InterfaceGetFontNumFromColVal( ulCol, 0 );
	mnPosInTextBuffer++;

	if ( mnPosInTextBuffer == MAX_STRINGS_IN_BUFFER )
	{
		mnPosInTextBuffer = 1;
//		PANIC_IF( FALSE, "Too many strings added to text buffer" )
	}
}






int	InterfaceTextRect( int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth, int nMaxHeight )
{
int		nNumLinesToUse = 0;
const char*	pcEndOfLine = szString;
int		nStartY;

	while ( pcEndOfLine != NULL )
	{
		pcEndOfLine = InterfaceTextLimitWidth( nLayer, nX, nY, (char*)pcEndOfLine, 0, font, nMaxWidth );
		nNumLinesToUse++;
	}
	
	nStartY = nY + nMaxHeight - ( nNumLinesToUse * GetStringHeight( szString, font ) );
	
	return( InterfaceTextBox( nLayer, nX, nStartY, szString, ulCol, font, nMaxWidth, TRUE ) );
}

int	InterfaceTextRectGetUsedWidth( const char* szString, int font, int nMaxWidth )
{
int		nWidth = GetStringWidth( szString, font );
	if ( nWidth > nMaxWidth ) nWidth = nMaxWidth;
	return( nWidth );
}

char* InterfaceTextLimitWidth( int nLayer, int nX, int nY, const char* szString, int ulCol, int nFont, int nMaxWidth )
{
char	acBuff[512];
int		nLoop = 0;
int		nBackstepcount = 0;
int		nPreBackstepPos;

	ZeroMemory(acBuff, 512 );
	acBuff[0] = szString[0];
	while ( ( acBuff[nLoop] != 0 ) &&
			( GetStringWidth( acBuff, nFont ) < nMaxWidth ) )
	{
		nLoop++;
		if ( nLoop == 250 )
		{
			break;
		}
		acBuff[nLoop] = szString[nLoop];
	}
	// If we got to the end of the string and it all fit, its fine to draw normally
	if ( acBuff[nLoop] == 0 )
	{
		if ( ulCol != 0 )
		{
			InterfaceText( nLayer, nX, nY, szString, ulCol, nFont );
		}
		return( NULL );
	}
	nPreBackstepPos = nLoop;
	while ( ( nLoop > 0 ) &&
			( nBackstepcount < 20 ) &&
			( acBuff[nLoop] != ' ' ) )
	{
		nLoop--;
		nBackstepcount++;
	}
	if ( acBuff[nLoop] != ' ' )
	{
		nLoop = nPreBackstepPos;
	}
	if ( nLoop == 0 )
	{
		/// Nothing will fit.. abort!!
		if ( ulCol != 0 )
		{
			InterfaceText( nLayer, nX, nY, acBuff, ulCol, nFont );
		}
		nLoop = strlen( szString );
	}
	else
	{
		// Otherwise, we need to chop the string..
		acBuff[nLoop] = 0;
		if ( ulCol != 0 )
		{
			InterfaceText( nLayer, nX, nY, acBuff, ulCol, nFont );
		}
	}
	return( (char*)szString + nLoop );
}

char* InterfaceTextLimitWidthCentred( int nLayer, int nX, int nY, const char* szString, int ulCol, int nFont, int nMaxWidth )
{
char	acBuff[512];
int		nLoop = 0;
int		nBackstepcount = 0;
int		nPreBackstepPos;

	ZeroMemory(acBuff, 512 );
	acBuff[0] = szString[0];
	while ( ( acBuff[nLoop] != 0 ) &&
			( GetStringWidth( acBuff, nFont ) < nMaxWidth ) )
	{
		nLoop++;
		if ( nLoop == 250 )
		{
			break;
		}
		acBuff[nLoop] = szString[nLoop];
	}
	// If we got to the end of the string and it all fit, its fine to draw normally
	if ( acBuff[nLoop] == 0 )
	{
		if ( ulCol != 0 )
		{
			InterfaceTextCentre( nLayer, nX, nY, szString, ulCol, nFont );
		}
		return( NULL );
	}
	nPreBackstepPos = nLoop;
	while ( ( nLoop > 0 ) &&
			( nBackstepcount < 20 ) &&
			( acBuff[nLoop] != ' ' ) )
	{
		nLoop--;
		nBackstepcount++;
	}
	if ( acBuff[nLoop] != ' ' )
	{
		nLoop = nPreBackstepPos;
	}
	if ( nLoop == 0 )
	{
		/// Nothing will fit.. abort!!
		if ( ulCol != 0 )
		{
			InterfaceTextCentre( nLayer, nX, nY, acBuff, ulCol, nFont );
		}
		nLoop = strlen( szString );
	}
	else
	{
		// Otherwise, we need to chop the string..
		acBuff[nLoop] = 0;
		if ( ulCol != 0 )
		{
			InterfaceTextCentre( nLayer, nX, nY, acBuff, ulCol, nFont );
		}
	}
	return( (char*)szString + nLoop );
}


void InterfaceText( int nLayer, int nX, int nY, const char* szString, uint32 ulCol, int nFont )
{

	if ( ( InterfaceIsSmall() == TRUE ) ||
		 ( ShouldAddString( szString ) == FALSE ) )
	{
		return;
	}

	AddFontString( nX , nY, szString, ulCol, mnCurrentFontFlags );
	if ( (uint32)(ulCol) > 0x1FF )
	{
		maxTextBuffer[ mnPosInTextBuffer - 1 ].bFont = (BYTE)(nFont);
	}
	maxTextBuffer[ mnPosInTextBuffer - 1 ].nLayer = nLayer;
}




/***************************************************************************
 * Function    : AddFontStringCenterLayer
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceTextCenter( int nLayer, int nX1, int nX2, int nY, const char* szString, uint32 ulCol, int nFont )
{
	if ( InterfaceIsSmall() == TRUE )
	{
		return;
	}
	if ( ShouldAddString( szString ) )
	{
		AddFontStringCenter( nY, nX1, nX2, szString, ulCol );
		if ( (uint32)(ulCol) < 0x1FF )
		{
			if ( ulCol < COL_FONT2 )
			{
				maxTextBuffer[ mnPosInTextBuffer-1 ].bFont = 0;
			}
			else if ( ulCol < COL_FONT3 )
			{
				maxTextBuffer[ mnPosInTextBuffer-1 ].bFont = 1;
			}
			else if ( ulCol < COL_FONT4 )
			{
				maxTextBuffer[ mnPosInTextBuffer-1 ].bFont = 2;
			}
			else
			{
				maxTextBuffer[ mnPosInTextBuffer-1 ].bFont = 3;
			}
		}
		else
		{
			maxTextBuffer[ mnPosInTextBuffer-1 ].bFont = (byte)(nFont);
		}
		maxTextBuffer[ mnPosInTextBuffer - 1 ].nLayer = nLayer;
	}
}


INTERFACE_API void	InterfaceTextCentre( int nLayer, int nX, int nY, const char* szString, uint32 ulCol, int font )
{
	InterfaceTextCenter( nLayer, nX - 200, nX + 200, nY, szString, ulCol, font );

}

INTERFACE_API void	InterfaceTxt( int nLayer, int nX, int nY, uint32 ulCol, int font, const char* text, ... )
{
char		acString[4096];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );
	if ( ulCol == 0 ) ulCol = 0xd0d0d0d0;			// Default col is an offwhite 

	InterfaceText( nLayer, nX, nY, acString, ulCol, font );
}


INTERFACE_API void	InterfaceTxtRight( int nLayer, int nX, int nY, uint32 ulCol, int font, const char* text, ... )
{
char		acString[4096];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );
	if ( ulCol == 0 ) ulCol = 0xd0d0d0d0;			// Default col is an offwhite 

	InterfaceTextRight( nLayer, nX, nY, acString, ulCol, font );

}

INTERFACE_API void	InterfaceTxtCentre( int nLayer, int nX, int nY, uint32 ulCol, int font, const char* text, ... )
{
char		acString[4096];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );

	if ( ulCol == 0 ) ulCol = 0xd0d0d0d0;			// Default col is an offwhite 

	InterfaceTextCentre( nLayer, nX, nY, acString, ulCol, font );
}



INTERFACE_API void	InterfaceTextScaled( int nLayer, int nX, int nY, const char* szString, uint32 ulCol, int font, float fSize )
{
TEXT_BUFFER*		pxText;
char*	pcTextBuffer;

	pxText = &maxTextBuffer[ mnPosInTextBuffer ];
	pcTextBuffer = pxText->acString;

	memcpy( pcTextBuffer, szString, strlen( szString ) + 1 );

	pxText->nX = nX;
	pxText->nY = nY;
	pxText->nCol = ulCol;
	pxText->nAlign = ALIGN_SCALED;
	pxText->fScale = fSize;
	pxText->nLayer = nLayer;
	pxText->wFlag = (short)(mnCurrentFontFlags);
	pxText->bFont = (BYTE)( InterfaceGetFontNumFromColVal( ulCol, font ) );
	mnPosInTextBuffer++;

	if ( mnPosInTextBuffer == MAX_STRINGS_IN_BUFFER )
	{
		mnPosInTextBuffer = 1;
//		PANIC_IF( FALSE, "Too many strings added to text buffer" )
	}

}

/***************************************************************************
 * Function    : InterfaceTextRight
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void InterfaceTextRight( int nLayer, int nX, int nY, const char* szString, uint32 ulCol, int nFont )
{
TEXT_BUFFER*		pxText;
char*	pcTextBuffer;
RECT		xAlignRect;

	if ( ( InterfaceIsSmall() == TRUE ) ||
		 ( ShouldAddString( szString ) == FALSE ) )
	{
		return;
	}

	pxText = &maxTextBuffer[ mnPosInTextBuffer ];

	xAlignRect.top = nY + mnInterfaceDrawY;
	xAlignRect.left = 0;
	xAlignRect.right = nX + mnInterfaceDrawX;

	pcTextBuffer = pxText->acString;

	memcpy( pcTextBuffer, szString, strlen( szString ) + 1 );

	pxText->nX = 0;
	pxText->nY = 0;
	pxText->nCol = ulCol;
	pxText->nAlign = ALIGN_RIGHT;
	pxText->xAlignRect = xAlignRect;
	pxText->nLayer = nLayer;
	pxText->wFlag = (short)(mnCurrentFontFlags);
	pxText->bFont = (BYTE)( InterfaceGetFontNumFromColVal( ulCol, nFont ) );

	mnPosInTextBuffer++;

	if ( mnPosInTextBuffer == MAX_STRINGS_IN_BUFFER )
	{
		mnPosInTextBuffer = 1;
//		PANIC_IF( FALSE, "Too many strings added to text buffer" )
	}

}


/***************************************************************************
 * Function    : GetFontU
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
float GetFontU( BYTE cChar, int nFont )
{
FONT_UVCHAR		uvChar;

	if ( InterfaceFontLookupChar( nFont, cChar, &uvChar ) == TRUE )
	{
		return( uvChar.u );
	}

	return( 0.0f );
}

float GetFontAdvance( BYTE cChar, int nFont )
{
FONT_UVCHAR		uvChar;
	if ( InterfaceFontLookupChar( nFont, cChar, &uvChar ) == TRUE )
	{
		return( uvChar.advance * mpFontDefs[nFont]->GetTextureSizeX() );
	}
	else 
	{
		return( GetFontUWidth( cChar, nFont ) );
	}
}


/***************************************************************************
 * Function    : GetFontUWidth
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
float GetFontUWidth( BYTE cChar, int nFont )
{
FONT_UVCHAR		uvChar;

	if ( InterfaceFontLookupChar( nFont, cChar, &uvChar ) == TRUE )
	{
		return( uvChar.w );
	}
	return( 0.0f );

}



int GetFontTextureHeight( int nFont )
{
	if ( ( nFont < MAX_FONTS_IN_GAME ) &&
		 ( mpFontDefs[nFont] ) )
	{
		return( mpFontDefs[nFont]->GetTextureSizeY() );
	}
	return( 256 );
}

int GetFontTextureWidth( int nFont )
{
	if ( ( nFont < MAX_FONTS_IN_GAME ) &&
		 ( mpFontDefs[nFont] ) )
	{
		return( mpFontDefs[nFont]->GetTextureSizeX() );
	}
	return( 256 );
}

/***************************************************************************
 * Function    : GetFontV
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
float GetFontV( BYTE cChar, int nFont )
{
float	fV = 0.0f;
FONT_UVCHAR		uvChar;

	if ( InterfaceFontLookupChar( nFont, cChar, &uvChar ) == TRUE )
	{
		return( uvChar.v );
	}
	return( fV );

}
float GetFontVHeight( BYTE cChar, int nFont )
{
float	fV = 0.0f;
FONT_UVCHAR		uvChar;

	if ( InterfaceFontLookupChar( nFont, cChar, &uvChar ) == TRUE )
	{
		return( uvChar.h );
	}
	return( fV );
}



float	FontGetCharUWidthAndScreenWidth( char cChar, int nFont, int nFlags, float* pfUWidth, int* pnScrWidth, int *pnAdvance )
{
int		nFontTextureWidth = GetFontTextureWidth(nFont);
float	fUWidth = GetFontUWidth( cChar, nFont );
int		nScrWidth = 0;
float	fScreenScaling = 1.0f;

	if ( fUWidth == 0 )
	{
		nScrWidth = 2;
	}
	else
	{
		nScrWidth = (int)( fUWidth * nFontTextureWidth );
	}

	if ( nFlags & FONT_FLAG_SMALL )
	{
		fScreenScaling = 0.75f;
	}
	else if ( nFlags & FONT_FLAG_TINY )
	{
		fScreenScaling = 0.25f;
	}
	else if ( nFlags & FONT_FLAG_GIANT )
	{
		fScreenScaling = mfGiantFontScale;
	}
	else if ( nFlags & FONT_FLAG_ENORMOUS )
	{
		fScreenScaling = 5.0f;
	}
	else if ( nFlags & FONT_FLAG_MASSIVE )
	{
		fScreenScaling = 3.1f;
	}
	else if ( nFlags & FONT_FLAG_LARGE )
	{
		fScreenScaling = 1.25f;
	}

	nScrWidth = (int)( nScrWidth * fScreenScaling );
	
	if ( pnAdvance )
	{
		// If its bottom aligned its a JSON sepc font, with advance separate from scrwidth
		if ( mpFontDefs[nFont]->IsBottomAligned() == TRUE )
		{
			*pnAdvance = (int)( GetFontAdvance( cChar, nFont ) * fScreenScaling );
		}
		else
		{
			*pnAdvance = nScrWidth;
		}
	}
	if ( pfUWidth )
	{
		*pfUWidth = fUWidth;
	}
	if ( pnScrWidth )
	{
		*pnScrWidth = nScrWidth;
	}

	return( fScreenScaling );
}


/***************************************************************************
 * Function    : FontDrawChar
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
int FontDrawChar( BYTE cChar, int nX, int nY, uint32 ulCol, int nFont, int nFlag, float fScaleOverride )
{
FLOATRECT	xScrRect;
FLOATRECT	xTexRect;
float		fFontVHeight;
int			nFontTextureWidth = GetFontTextureWidth(nFont);
int			nFontScreenWidth;
int			nFontScreenHeight;
float		fCharUWidth;
FONT_UVCHAR		uvChar;
BOOL			bHasUVChar = FALSE;
float		fTextureHeightScale = 1.0f;
BOOL			bIsBottomAlignedFont = FALSE;
float		fScreenScaling;
float		fAlignScale = 1.0f;
CFontDef*		pFontDef = mpFontDefs[nFont];
int			nAdvance = 0;

	bHasUVChar = InterfaceFontLookupChar( nFont, cChar, &uvChar );
	if ( pFontDef->IsBottomAligned() )
	{
		bIsBottomAlignedFont = TRUE;
	}

	switch( cChar )
	{
	case '\t':
		if ( InterfaceFontIsFixedWidth( nFont ) == TRUE )
		{
			return( 4*10 );
		}
		return( 4*12 );
	case 163:		// £
		break;
	case '\n':
	case '\r':
		return( 0 );
	default:
		if ( ( cChar > 140 ) || 
			 ( cChar < 32 ) )
		{
			return( 0 );
		}
		break;
	}
	 
	fScreenScaling = FontGetCharUWidthAndScreenWidth( cChar, nFont, nFlag, &fCharUWidth, &nFontScreenWidth, &nAdvance );
	fFontVHeight = GetFontVHeight(cChar,nFont);

	if ( nFlag & FONT_FLAG_SMALL )
	{
		fTextureHeightScale =  GetFontTextureHeight(nFont) * 0.75f;
	}
	else if ( nFlag & FONT_FLAG_TINY )
	{
		fTextureHeightScale =  GetFontTextureHeight(nFont) * 0.25f;
		fAlignScale = 0.25f;
	}
	else if ( nFlag & FONT_FLAG_GIANT )
	{
		fTextureHeightScale =  GetFontTextureHeight(nFont) * mfGiantFontScale;
	}
	else if ( nFlag & FONT_FLAG_ENORMOUS )
	{
		fTextureHeightScale =  GetFontTextureHeight(nFont) * 5.0f;
	}
	else if ( nFlag & FONT_FLAG_MASSIVE )
	{
		fTextureHeightScale =  GetFontTextureHeight(nFont) * 3.1f;
	}
	else if ( nFlag & FONT_FLAG_LARGE )
	{
		fTextureHeightScale =  GetFontTextureHeight(nFont) * 1.25f;
	}
	else
	{
		fTextureHeightScale = (float)GetFontTextureHeight(nFont);
	}
	nFontScreenHeight = (int)( fFontVHeight * fTextureHeightScale);

	xTexRect.x1 = GetFontU( cChar, nFont );
	xTexRect.x2 = xTexRect.x1 + fCharUWidth;
	if ( xTexRect.x2 < xTexRect.x1 )
	{
		xTexRect.x2 = 1.0f;
	}

	xTexRect.y1 = GetFontV( cChar, nFont );
	xTexRect.y2 = xTexRect.y1 + fFontVHeight;

	if ( fScaleOverride > 0.0f )
	{
		nFontScreenWidth = (int)( nFontScreenWidth * fScaleOverride );
		nFontScreenHeight = (int)( nFontScreenHeight * fScaleOverride );
		nAdvance = (int)( nAdvance * fScaleOverride );
	}
	else
	{
		fScaleOverride = 1.0f;
	}

	xScrRect.x1 = (float)( nX );
	xScrRect.y1 = (float)( nY );

	if ( bIsBottomAlignedFont )
	{
		if ( bHasUVChar )
		{
			xScrRect.y1 += (pFontDef->mnFontOverallSize + (pFontDef->mnFontOccupyHeightReduction*2)) * (fScaleOverride * fAlignScale);		// Hack.. should be font size...
			xScrRect.y1 -= (uvChar.originY * fTextureHeightScale) * fScaleOverride;

			xScrRect.y1 -= 20 * (fScaleOverride * fAlignScale);
		}
	}

	xScrRect.x2 = xScrRect.x1 + (float)(nFontScreenWidth);
	xScrRect.y2 = xScrRect.y1 + (float)(nFontScreenHeight);

	// If bold, add an extra big char
	if ( nFlag & FONT_FLAG_BOLD )
	{
	FLOATRECT	xScrRect2 = xScrRect;
		xScrRect2.x1 -= 1.0f;
		xScrRect2.y1 -= 1.0f;
		AddCharVertices( &xScrRect, &xTexRect, ulCol, nFlag );
	}
	if ( nFlag & FONT_FLAG_DROP_SHADOW )
	{
	FLOATRECT	xScrRect2 = xScrRect;
	float		fMainAlpha = (float)( ulCol >> 24 ) / 255.0f; 
	uint32		ulShadowCol;

		if ( nFlag & FONT_FLAG_GIANT )
		{
			xScrRect2.x1 += 2.0f;
			xScrRect2.y1 += 2.0f;
			xScrRect2.x2 += 2.0f;
			xScrRect2.y2 += 2.0f;
		}
		else if ( nFlag & FONT_FLAG_ENORMOUS )
		{
			xScrRect2.x1 += 4.0f;
			xScrRect2.y1 += 4.0f;
			xScrRect2.x2 += 4.0f;
			xScrRect2.y2 += 4.0f;
		}
		else if ( nFlag & FONT_FLAG_MASSIVE )
		{
			xScrRect2.x1 += 3.0f;
			xScrRect2.y1 += 3.0f;
			xScrRect2.x2 += 3.0f;
			xScrRect2.y2 += 3.0f;
		}
		else if ( nFlag & FONT_FLAG_LARGE )
		{
			xScrRect2.x1 += 2.0f;
			xScrRect2.y1 += 2.0f;
			xScrRect2.x2 += 2.0f;
			xScrRect2.y2 += 2.0f;
		}
		else if ( nFlag & FONT_FLAG_SMALL )
		{
			xScrRect2.x1 += 1.0f;
			xScrRect2.y1 += 1.0f;
			xScrRect2.x2 += 1.0f;
			xScrRect2.y2 += 1.0f;
		}
		else if ( nFlag & FONT_FLAG_TINY )
		{
			xScrRect2.x1 += 1.0f;
			xScrRect2.y1 += 1.0f;
			xScrRect2.x2 += 1.0f;
			xScrRect2.y2 += 1.0f;
		}
		else
		{
			xScrRect2.x1 += 2.0f;
			xScrRect2.y1 += 2.0f;
			xScrRect2.x2 += 2.0f;
			xScrRect2.y2 += 2.0f;
		}

		ulShadowCol = GetColWithModifiedAlpha( 0xA0000000, fMainAlpha );
		AddCharVertices( &xScrRect2, &xTexRect, ulShadowCol, nFlag );
	}
	AddCharVertices( &xScrRect, &xTexRect, ulCol, nFlag );

	if ( InterfaceFontIsFixedWidth( nFont ) == TRUE )
	{
		return( 10 );
	}

//	if ( uvChar.advance > 0 )
	if ( ( cChar == 32 ) &&
		 ( uvChar.advance > 0 ) )
	{
		return( (int)( (uvChar.advance * nFontTextureWidth) * fScreenScaling ) );
	}

	return( (nAdvance + 1) - mpFontDefs[nFont]->mnFontOccupyWidthReduction );

}




/***************************************************************************
 * Function    : ChopTextWidth
 * Params      : String, pixel width, colour
 * Returns     : Number of chars that will fit in the screen width passed
 * Description : 
 ***************************************************************************/
int ChopTextWidth( char* pcString, int nWidth, int nFont )
{
int		nX;
int		nChar;
int		nCount;
int		nFontScreenWidth = 0;
int		nAdvance = 0;

	FontGetCharUWidthAndScreenWidth( *pcString, nFont, mnCurrentFontFlags, NULL, &nFontScreenWidth, &nAdvance );

	nX = nFontScreenWidth + 1;
	nCount = 0;

	while ( ( *(pcString) != 0 ) &&
		    ( nX < nWidth ) &&
		    ( nCount < 8192 ) )	// see MAX_CHARS_IN_EDIT_STRING
	{
		nChar = *(pcString);
		FontGetCharUWidthAndScreenWidth( nChar, nFont, mnCurrentFontFlags, NULL, &nFontScreenWidth, &nAdvance );
		nX += nFontScreenWidth + 1;
	
		pcString++;
		nCount++;
	}

	return( nCount );

}




/***************************************************************************
 * Function    : GetStringWidth
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
int GetStringWidth( const char* pcString, int nFont )
{
	if ( pcString )
	{
	int		nX;
	BYTE	cChar;
	int		nCount;
	int		nFontScreenWidth = 0;
	int		nAdvance = 0;

		nX = 0;
		nCount = 0;

		while ( ( *(pcString) != 0 ) &&
				 ( nCount < 8192 ) )	// see MAX_CHARS_IN_EDIT_STRING
		{
			cChar = *(pcString);
			switch ( cChar )
			{
			case '\t':
				if ( InterfaceFontIsFixedWidth( nFont ) == TRUE )
				{
					nX += ( 4*10 );
				}
				else
				{
					nX += (4*12);
				}
				break;
			case 250:
				nX += (GetStringHeight( "A", nFont ) + 4 );
				pcString++;
				nCount++;
				break;
			default:
				// If valid tag
				if ( ( cChar == '<' ) &&
					 ( pcString[1] != 0 ) &&
					 ( pcString[1] != ' ' ) )
				{
				const char* pcTagStart = pcString;
				int			nTagStartCount = nCount;
				int			nTagCount = 0;

					pcString++;
					while ( ( *(pcString) != 0 ) &&
							( nCount < 8192 ) )	// see MAX_CHARS_IN_EDIT_STRING
					{
						if ( ( *pcString == '>' ) ||
							 ( nTagCount >= 64 )) 
						{
							break;
						}
						pcString++;
						nTagCount++;
						nCount++;
					}

					if ( *pcString != '>' )
					{
						pcString = pcTagStart;
						nCount = nTagStartCount;
						FontGetCharUWidthAndScreenWidth( cChar, nFont, mnCurrentFontFlags, NULL, &nFontScreenWidth, &nAdvance );
						nX += (nAdvance + 1);
					}
				}
				else
				{
					FontGetCharUWidthAndScreenWidth( cChar, nFont, mnCurrentFontFlags, NULL, &nFontScreenWidth, &nAdvance );
					nX += (nAdvance + 1);
				}
				break;
			}

			pcString++;
			nCount++;
		}

		return( nX );
	}
	else
	{
		return( 0 );
	}
}


int GetStringHeight( const char* pcString, int nFont )
{
int		nChar;
int		nHeight = 0;

	nChar = *(pcString);
	if ( nChar == 0 ) nChar = 'A';

	if ( mnCurrentFontFlags & FONT_FLAG_SMALL )
	{
		nHeight = (int)( GetFontVHeight( nChar, nFont ) * GetFontTextureHeight(nFont) * 0.75f ) + 1;
	}
	else if ( mnCurrentFontFlags & FONT_FLAG_TINY )
	{
		nHeight = (int)( GetFontVHeight( nChar, nFont ) * GetFontTextureHeight(nFont) * 0.25f ) + 1;
	}
	else if ( mnCurrentFontFlags & FONT_FLAG_GIANT )
	{
		nHeight = (int)( GetFontVHeight( nChar, nFont ) * GetFontTextureHeight(nFont) * mfGiantFontScale ) + 1;
	}
	else if ( mnCurrentFontFlags & FONT_FLAG_ENORMOUS )
	{
		nHeight = (int)( GetFontVHeight( nChar, nFont ) * GetFontTextureHeight(nFont) * 5.0f ) + 1;
	}
	else if ( mnCurrentFontFlags & FONT_FLAG_MASSIVE )
	{
		nHeight = (int)( GetFontVHeight( nChar, nFont ) * GetFontTextureHeight(nFont) * 3.1f ) + 1;
	}
	else if ( mnCurrentFontFlags & FONT_FLAG_LARGE )
	{
		nHeight = (int)( GetFontVHeight( nChar, nFont ) * GetFontTextureHeight(nFont) * 1.25f ) + 1;
	}
	else 
	{
		nHeight = (int)( GetFontVHeight( nChar, nFont ) * GetFontTextureHeight(nFont) ) + 1;
	}

	return( nHeight );

}

int		InterfaceTextBox( int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth, BOOL bLeftAlign )
{
char*	pcEndOfLine = (char*)szString;
int		nLineSep = 13;
int		nBaseY = nY;
int		nStringWidth;

	nLineSep = (int)( GetFontVHeight( 'A', font ) * GetFontTextureHeight(font)) + 1;

	if ( mnCurrentFontFlags & FONT_FLAG_SMALL )
	{
		nLineSep = (int)( nLineSep * 0.75f );
	}
	else if ( mnCurrentFontFlags & FONT_FLAG_TINY )
	{
		nLineSep = (int)( nLineSep * 0.25f );
	}
	else if ( mnCurrentFontFlags & FONT_FLAG_GIANT )
	{
		nLineSep = (int)( nLineSep * 2.0f );
	}
	else if ( mnCurrentFontFlags & FONT_FLAG_ENORMOUS )
	{
		nLineSep = (int)( nLineSep * 5.0f );
	}
	else if ( mnCurrentFontFlags & FONT_FLAG_MASSIVE )
	{
		nLineSep = (int)( nLineSep * 3.1f );
	}
	else if ( mnCurrentFontFlags & FONT_FLAG_LARGE )
	{
		nLineSep = (int)( nLineSep * 1.25f );
	}


	nStringWidth = GetStringWidth( szString, font );

	// If whole text fits on a single line
	if ( nStringWidth < nMaxWidth )
	{
		if ( bLeftAlign )
		{
			InterfaceText( nLayer, nX, nY, szString, ulCol, font );
		}
		else
		{
			InterfaceTextCenter( nLayer, nX, nX + nMaxWidth, nY, szString, ulCol, font );
		}
		nY += nLineSep;
	}
	else
	{
		if ( bLeftAlign )
		{
			pcEndOfLine = InterfaceTextLimitWidth( nLayer, nX, nY, (char*)pcEndOfLine, ulCol, font, nMaxWidth );
		}
		else
		{
			pcEndOfLine = InterfaceTextLimitWidthCentred( nLayer, nX + (nMaxWidth/2), nY, (char*)pcEndOfLine, ulCol, font, nMaxWidth );
		}
		nY += nLineSep;
		while ( pcEndOfLine != NULL )
		{
			nStringWidth = GetStringWidth( pcEndOfLine, font );

			// If whole text fits on a single line
			if ( ( bLeftAlign == FALSE ) &&
				 ( nStringWidth < nMaxWidth ) )
			{
				InterfaceTextCenter( nLayer, nX, nX + nMaxWidth, nY, pcEndOfLine, ulCol, font );		
				pcEndOfLine = NULL;
			}
			else
			{
				pcEndOfLine = InterfaceTextLimitWidth( nLayer, nX, nY, (char*)pcEndOfLine, ulCol, font, nMaxWidth );
			}
			nY += nLineSep;
		}
	}
	return( nY - nBaseY );
}	


int	InterfaceTextGetWidth( const char* pcString, int nFont )
{
	return( GetStringWidth( pcString, nFont ) );
}

int	InterfaceTextGetHeight( const char* pcString, int nFont )
{
	return( GetStringHeight( pcString, nFont ) );
}

InterfaceControllerIconCallback		mfnControllerIconCallback = NULL;


void	InterfaceSetControllerIconFunction( InterfaceControllerIconCallback fnControllerIconCallback )
{
	mfnControllerIconCallback = fnControllerIconCallback;
}

/***************************************************************************
 * Function    : FontDrawText
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
int FontDrawText( char* pcString, RECT* pxAlignRect, int nAlign, uint32 ulCol, int nFont, int nFlag, float fTextScale )
{
int		nX;
int		nY;
int		nWidth, nHeight;
int		nSize;
int		nCount;
BYTE	cChar;

	switch( nAlign )
	{
	case ALIGN_CENTRE:
		nWidth = GetStringWidth( pcString, nFont );

		nSize = pxAlignRect->right - pxAlignRect->left;
		nSize /= 2;
		nSize -= (nWidth / 2);

		nX = pxAlignRect->left + nSize;
		break;
	case ALIGN_RIGHT:
		nWidth = GetStringWidth( pcString, nFont );
		nX = pxAlignRect->right - nWidth;
		break;
	case ALIGN_SCALED:
		nWidth = GetStringWidth( pcString, nFont );
		nWidth = (int)( nWidth * fTextScale );
		nHeight = GetStringHeight( pcString, nFont );
		nHeight = (int)( nHeight * fTextScale );

		nX = pxAlignRect->left - (nWidth / 2);
		pxAlignRect->top = pxAlignRect->top - (nHeight / 2);
		break;

	case ALIGN_LEFT:
	default:
		nX = pxAlignRect->left;
		break;
	}

	nY = pxAlignRect->top + mpFontDefs[nFont]->mnFontDrawOffsetY;
	nX += mpFontDefs[nFont]->mnFontDrawOffsetX;

	nCount = 0;
	while ( ( *(pcString) != 0 ) &&
		    ( nCount < 300 ) )
	{
		cChar = *( (BYTE*)( pcString ));
		if ( cChar == 250 )
		{
			if ( mfnControllerIconCallback )
			{
			int		nIconTexture;
			int		nHeight = GetStringHeight( "A", nFont ) + 4;
			int		nSize = 0;
				pcString++;
				nCount++;

				if ( nHeight > 32 )
				{
					nSize = 1;
				}
				int		nControllerActionNum = *( (BYTE*)( pcString ));
				nIconTexture = mfnControllerIconCallback( NOTFOUND, nControllerActionNum, nSize );

				if ( nIconTexture != NOTFOUND )
				{
				int		nOverlay = InterfaceCreateNewTexturedOverlay( 2, nIconTexture );

					InterfaceTexturedRect( nOverlay, nX, nY - 2, nHeight, nHeight, 0xE0FFFFFF, 0.0f, 0.0f, 1.0f, 1.0f );
					nX += nHeight;
				}
			}
		}
		else
		{
			nX += FontDrawChar( cChar, nX, nY, ulCol, nFont, nFlag, fTextScale );
		}
		pcString++;
		nCount++;
	}

	return( nX - pxAlignRect->left );
}



/***************************************************************************
 * Function    : ClearStrings
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void ClearStrings( void )
{
	mnPosInTextBuffer = 0;

}


void InitialiseFontBuffers( void )
{
#ifdef TUD9
	InitialiseFontBuffersDX();
#endif
#ifdef USING_OPENGL
	InitialiseFontBuffersGL();
#endif

}

/***************************************************************************
 * Function    : InitialiseFont
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
HRESULT InitialiseFont( BOOL bUseDefaultFonts )
{
	InitialiseFontBuffers();

	if ( bUseDefaultFonts )
	{
		// load a default font using the fontrast generated stuff
		InterfaceFontLoad( 0, "Data/Fonts/sansserif-8b000.bmp", "Data/Fonts/sansserif-8b.txt", 0 );
	//	InterfaceFontLoad( 0, "Data/Textures/Font/arialbold000.bmp", "Data/Textures/Font/arialbold.txt", 0 );
		InterfaceFontLoad( 1, "Data/Fonts/arialboldlarge000.bmp", "Data/Fonts/arialboldlarge.txt", 0 );
	//	InterfaceFontLoad( 2, "Data/Textures/Font/space1000.bmp", "Data/Textures/Font/space1.txt", 0 );
		InterfaceFontLoad( 2, "Data/Fonts/nina-10b000.bmp", "Data/Fonts/nina-10b.txt", 0 );
	//	InterfaceFontLoad( 3, "Data/Textures/Font/space2000.bmp", "Data/Textures/Font/space2.txt", 0 );
	//	InterfaceFontLoad( 3, "Data/Textures/Font/mssansserif-12000.bmp", "Data/Textures/Font/mssansserif-12.txt", 0 );
		InterfaceFontLoad( 3, "Data/Fonts/franklingothic-8b000.bmp", "Data/Fonts/franklingothic-8b.txt", 0 );
		InterfaceFontLoad( 4, "Data/Fonts/TitilliumWeb64-01.png", "Data/Fonts/TitilliumWeb64-01.json", 0 );
	}

	return( 0 );

}






