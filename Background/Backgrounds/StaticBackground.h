#ifndef DISPLAY_BACKGROUNDS_STATIC_BACKGROUND_H
#define DISPLAY_BACKGROUNDS_STATIC_BACKGROUND_H

#include "../Background.h"

class StaticBackground : public BackgroundBase
{
public:
	StaticBackground()
	{
		mhBackgroundTexture = NOTFOUND;
	}

	virtual void	OnInitialise( void );

	virtual void	OnUpdate( float fDelta ) {}

	virtual void	OnDisplay( float fGlobalAlpha );

	virtual void	OnShutdown( void );

private:
	int		mhBackgroundTexture;

};



#endif