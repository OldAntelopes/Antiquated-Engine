
#ifndef FONT_FONT_H
#define FONT_FONT_H


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
	ALIGN_RIGHT
};

#define MAX_CHARS_IN_STRING			256
#define MAX_STRINGS_IN_BUFFER		2048

typedef struct 
{
	int		nX;
	int		nY;
	int		nCol;
	char	acString[ MAX_CHARS_IN_STRING ];

	RECT	xAlignRect;

	short	nAlign;
	short	nLayer;
	short	wFlag;
	BYTE	bFont;
	BYTE	bPad1;
	
} TEXT_BUFFER;




extern HRESULT InitialiseFont( void );
extern void FreeFont( void );
extern void AddFontStringAlign( char* szString, int, int, RECT* );
extern void ClearStrings( void );

extern float GetFontU( BYTE cChar, int nFont );
extern float GetFontV( BYTE cChar, int nFont );
extern float GetFontUWidth( BYTE cChar, int nFont );
extern float GetFontAdvance( BYTE cChar, int nFont );

extern BOOL	InterfaceFontIsFixedWidth( int nFontNum );


#ifdef __cplusplus
}
#endif


#endif