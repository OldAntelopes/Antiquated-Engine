#ifndef FONT_COMMON_H
#define FONT_COMMON_H

#include "../../DirectX/InterfaceTypesDX.h"

#define		MAX_FONTS_IN_GAME		8
#define		SIZE_OF_FONT_VERTEX_BUFFER		1020

typedef struct
{
	float	x1;	
	float	x2;	
	float	y1;	
	float	y2;	

} FLOATRECT;

//------------------------------------------------------------------------------------------------------
class CFontDef
{
public:
	CFontDef()
	{
		m_TextureSizeX = 0;
		m_TextureSizeY = 0;
		mhTexture = NOTFOUND;
#ifdef TUD9
		mpTexture = NULL;
#endif
		m_bFilteringOn = TRUE;
		m_bIsFixedWidth = FALSE;
		m_bIsBottomAligned = FALSE;
		m_szTextureFilename[0] = 0;
		mnFontOccupyWidthReduction = 0;
		mnFontOccupyHeightReduction = 0;
		mnFontOverallSize = 32;
		mnFontDrawOffsetX = 0;
		mnFontDrawOffsetY = 0;
	}
	~CFontDef()
	{
		FreeTexture();
	}
	void 	Initialise( const char* szDefName );

	void	SetTextureFileName( const char* szTextureFilename )
	{
		strcpy( m_szTextureFilename, szTextureFilename );
	}
	void 	LoadTexture( InterfaceInstance* pInterfaceInstance );


	void	LookupChar( BYTE cChar, FONT_UVCHAR* pOut )
	{
		*pOut = maFontLookup[cChar];
	}
	int		GetTextureSizeX( void ) { return m_TextureSizeX; }
	int		GetTextureSizeY( void ) { return m_TextureSizeY; }

	void	FreeTexture( void )
	{
		if ( mhTexture != NOTFOUND )
		{
			InterfaceReleaseTexture( mhTexture );
			mhTexture = NOTFOUND;
		}
#ifdef TUD9
		if ( mpTexture != NULL )
		{
			mpTexture->Release();
			mpTexture = NULL;
		}
#endif
	}

	void	SetTextureAsCurrent( InterfaceInstance* );

	void	EnableFiltering( BOOL bEnable ) { m_bFilteringOn = bEnable; }
	void	SetFixedWidth( BOOL bEnable, int nWidth ) { m_bIsFixedWidth = bEnable; }
	BOOL	IsFilteringOn( void ) { return( m_bFilteringOn ); }
	BOOL	IsFixedWidth( void ) { return( m_bIsFixedWidth ); }
	BOOL	IsBottomAligned( void ) { return( m_bIsBottomAligned ); }

	int			mnFontOccupyWidthReduction;
	int			mnFontOccupyHeightReduction;
	int			mnFontOverallSize;
	int			mnFontDrawOffsetX;
	int			mnFontDrawOffsetY;

private:
	void			ReadFontrastFile( const char* szFontDefinitionFilename );
	void			ReadJSONLayoutFile( const char* szFontDefinitionFilename );

	FONT_UVCHAR		maFontLookup[256];
	int			mhTexture; 
#ifdef TUD9
	LPGRAPHICSTEXTURE	 mpTexture;
#endif
	int			m_TextureSizeX;
	int			m_TextureSizeY;
	char		m_szTextureFilename[256];
	BOOL		m_bFilteringOn;
	BOOL		m_bIsFixedWidth;
	BOOL		m_bIsBottomAligned;

};



#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	

enum
{
	FONT_TYPE_1 = 0,
	FONT_TYPE_2,
	FONT_TYPE_3,
	FONT_TYPE_4,
	MAX_FONT_TYPES,
};

enum
{
	ALIGN_LEFT = 0,
	ALIGN_CENTRE,
	ALIGN_RIGHT,
	ALIGN_SCALED,
};

#define MAX_CHARS_IN_STRING			256
#define MAX_STRINGS_IN_BUFFER		4096

typedef struct 
{
	int		nX;
	int		nY;
	uint32	nCol;
	float	fScale;

	char	acString[ MAX_CHARS_IN_STRING ];

	RECT	xAlignRect;
	
	short	nAlign;
	short	nLayer;
	short	wFlag;
	BYTE	bFont;
	BYTE	bPad1;
	
} TEXT_BUFFER;





extern void AddFontStringAlign( char* szString, int, int, RECT* );

extern float GetFontU( BYTE cChar, int nFont );
extern float GetFontV( BYTE cChar, int nFont );
extern float GetFontUWidth( BYTE cChar, int nFont );

extern BOOL	InterfaceFontIsFixedWidth( int nFontNum );
extern BOOL	InterfaceFontIsFilteringOn( int nFontNum );


extern HRESULT InitialiseFont( BOOL bUseDefaultFonts );

extern void InitialiseFontBuffers( void );

extern int GetFontTextureHeight( int nFont );

extern int					mnCurrentFontFlags;
extern TEXT_BUFFER			maxTextBuffer[];
extern FLATVERTEX*			mpFontVertices;
extern int		mnPosInTextBuffer;

extern int FontDrawText( char* pcString, RECT* pxAlignRect, int nAlign, uint32 ulCol, int nFont, int nFlag, float fTextScale );
extern void	FontDefFreeAll( void );

#ifdef __cplusplus
}
#endif

class FontSystem : public InterfaceModule
{
public:
	BOOL	LoadFont( int nFontNum, const char* pcImageFileName, const char* pcLayoutFile, uint32 ulFlags );
	void	InitialiseFontBuffers( void );
	HRESULT	InitialiseFonts( BOOL bUseDefaultFonts );
	
	void	Text( int nLayer, int nX, int nY, const char* szString, uint32 ulCol, int nFont );
	void	TextCentre( int nLayer, int nX1, int nX2, int nY, const char* szString, uint32 ulCol, int nFont );

	void	RenderStrings( int nLayer );
	void	ClearStrings( void );
	void	FreeResources( BOOL bFreeEverything );

	BOOL	IsFontFixedWidth( int nFontNum );
	BOOL	IsFontFilteringOn( int nFontNum );
	BOOL	FontSetAsCurrentTexture( int nFontNum );
	BOOL	FontLookupChar( int nFontNum, char cChar, FONT_UVCHAR* pOut );
	BOOL	FontSetFixedOffsets( int nFontNum, int nPosOffsetX, int nPosOffsetY, int nOccupyWidthReduction, int nOccupyHeightReduction );
	void	FreeFont( int nFontNum );
	BOOL	IsFontLoaded( int nFontNum );
	int		ChopTextWidth( char* pcString, int nWidth, int nFont );
	int		GetStringWidth( const char* pcString, int nFont );
	int		GetStringHeight( const char* pcString, int nFont );
	int		TextBoxMaxHeight(int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth, int nMaxHeight, BOOL bLeftAlign);

private:
	void	InitialiseFontBuffersDX( void );

	void	StringRenderBegin( void );

	void	AddFontString( int nX, int nY, const char* szString, uint32 ulCol, int nFlag );
	void	AddFontStringCenter( int nY, int nX1, int nX2, const char* szString, uint32 ulCol );

	int		FontDrawText( char* pcString, RECT* pxAlignRect, int nAlign, uint32 ulCol, int nFont, int nFlag, float fTextScale );
	void	AddCharVertices( FLOATRECT* pxScrRect, FLOATRECT* pxTexRect, uint32 ulCol, int nFlag );
	float	FontDrawChar( BYTE cChar, int nX, int nY, uint32 ulCol, int nFont, int nFlag, float fScaleOverride );
	float	FontGetCharUWidthAndScreenWidth( char cChar, int nFont, int nFlags, float* pfUWidth, int* pnScrWidth, float *pfAdvance );
	float	GetFontAdvance( BYTE cChar, int nFont );
	int		GetFontTextureWidth( int nFont );
	int		GetFontTextureHeight( int nFont );
	float	GetFontVHeight( BYTE cChar, int nFont );
	float	GetFontV( BYTE cChar, int nFont );
	float	GetFontU( BYTE cChar, int nFont );
	float	GetFontUWidth( BYTE cChar, int nFont );

	void	DrawFontBufferDX( void );
	void	FontDefFreeAll( void );

	int							mnCurrentRenderFont = 0;

	IGRAPHICSVERTEXBUFFER*		mpxFontVertexBuffer1 = NULL;
	IGRAPHICSVERTEXBUFFER*		mpxFontVertexBuffer2 = NULL;
	IGRAPHICSVERTEXBUFFER*		mpxCurrentFontVertexBuffer = NULL;

	CFontDef*	mpFontDefs[MAX_FONTS_IN_GAME] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

	FLATVERTEX*			mpFontVertices = NULL;
	int					mnFontVertexIndex = 0;
};

#endif