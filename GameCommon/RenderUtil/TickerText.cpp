
#include "StandardDef.h"
#include "Interface.h"

#include "TickerText.h"



TickerText::TickerText()
{
	
	mszText = NULL;
	mfRevealSpeed = 1.0f;
	mfElapsedTime = 0.0f;
	
	mnNumCharsRevealed = 0;
	mnTextLen = 0;

}


TickerText::~TickerText()
{
	SAFE_FREE( mszText );
}


void	TickerText::Initialise( const char* szString, float fRevealSpeed )
{
	mnTextLen = strlen( szString );
	mszText = (char*)( malloc( mnTextLen + 1 ) );
	strcpy( mszText, szString );
	mnNumCharsRevealed = 0;
	mfRevealSpeed = fRevealSpeed;
}

void	TickerText::Update( float fDelta )
{
	mfElapsedTime += fDelta * mfRevealSpeed;

	mnNumCharsRevealed = (int)( mfElapsedTime );
	if ( mnNumCharsRevealed > mnTextLen ) 
	{
		mnNumCharsRevealed = mnTextLen;
	}
}


void	TickerText::Render( int X, int Y, int W, float fAlpha, uint32 ulCol, int nFont, int nFlags )
{
char		acString[512];
int			nLoop;

	for( nLoop = 0; nLoop < mnNumCharsRevealed; nLoop++ )
	{
		acString[nLoop] = mszText[nLoop];
	}
	acString[nLoop] = 0;

	if ( nFlags == 1 )
	{
	int		nStringWidth = GetStringWidth( mszText, nFont );
	int		nOffset;

		nOffset = (W - nStringWidth) / 2;
		InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );
		InterfaceText( 1, X + nOffset, Y, acString, ulCol, nFont );
		InterfaceSetFontFlags( 0 );
	}
	else
	{
		InterfaceText( 1, X, Y, acString, ulCol, nFont );
	}

}




