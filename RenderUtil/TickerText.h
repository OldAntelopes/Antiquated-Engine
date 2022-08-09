#ifndef RENDERUTIL_TICKER_TEXT_H
#define RENDERUTIL_TICKER_TEXT_H




class TickerText
{
public:
	TickerText();
	~TickerText();

	void		Initialise( const char* szString, float fRevealSpeed );

	void		Update( float fDelta );
	void		Render( int X, int Y, int W, float fAlpha, ulong ulCol, int nFont, int nFlags );

private:

	char*		mszText;

	float		mfRevealSpeed;
	float		mfElapsedTime;
	
	int			mnNumCharsRevealed;
	int			mnTextLen;

};










#endif