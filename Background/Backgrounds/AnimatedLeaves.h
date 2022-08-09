#ifndef DISPLAY_BACKGROUNDS_ANIMATED_LEAVES_H
#define DISPLAY_BACKGROUNDS_ANIMATED_LEAVES_H

#include "../Background.h"

class AnimatedLeaves : public BackgroundBase
{
public:
	enum
	{
		NUM_SPRITES = 4,
	};

	AnimatedLeaves()
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

	virtual void	OnShutdown( void );

private:
	int		mhBackgroundTexture;

	int		mahBackgroundSprites[NUM_SPRITES];

};






#endif