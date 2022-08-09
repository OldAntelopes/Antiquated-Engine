#ifndef DISPLAY_BACKGROUNDS_SNOWFLAKES_H
#define DISPLAY_BACKGROUNDS_SNOWFLAKES_H

#include "../Background.h"

class Snowflakes : public BackgroundBase
{
public:
	enum
	{
		NUM_SPRITES = 3,
	};

	Snowflakes()
	{
	int		nLoop;

		mhBackgroundTexture = NOTFOUND;
		for( nLoop = 0; nLoop < NUM_SPRITES; nLoop++ )
		{
			mahBackgroundSprites[nLoop] = NOTFOUND;
		}
	}

	virtual void	OnInitialise( void );

	virtual void	OnUpdate( float fDelta );

	virtual void	OnDisplay( float fGlobalAlpha );

	virtual void	OnDisplayUpperLayer( void );

	virtual void	OnShutdown( void );

private:
	int		mhBackgroundTexture;

	int		mahBackgroundSprites[NUM_SPRITES];

};






#endif