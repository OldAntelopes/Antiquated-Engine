#ifndef STRING_LIST_H
#define STRING_LIST_H

#define MAX_STRINGS_IN_BUFFER		1024

enum
{
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_CENTER,
	ALIGN_BOX,
	ALIGN_RECT,
	ALIGN_SCALED,
	ALIGN_BOX_CENTRE,
};


typedef struct 
{
	int		nX;
	int		nY;
	uint32	ulCol;
	char*	pcString;

	int		nWidth;
	int		nHeight;

	short	wAlign;
	short	wLayer;

	short	wFlag;
	BYTE	bFont;
	BYTE	bPad1;

	float	fScale;
	
} TEXT_BUFFER;


extern TEXT_BUFFER*		StringListGetNext( int nLayer, TEXT_BUFFER* pLast );

extern void				StringListInit( void );
extern void				StringListReset( void );
extern void				StringListShutdown( void );

extern int		mnCurrentFontFlags;

#endif