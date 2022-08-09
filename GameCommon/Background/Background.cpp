
#include "StandardDef.h"
#include "Interface.h"

#include "Backgrounds\AnimatedLeaves.h"
#include "Backgrounds\StaticBackground.h"
#include "Backgrounds\Snowflakes.h"
#include "Background.h"

BackgroundBase*		mspBackgroundClass = NULL;

void		BackgroundInitialise( void )
{
	mspBackgroundClass = new AnimatedLeaves;
//	mspBackgroundClass = new StaticBackground;
//	mspBackgroundClass = new Snowflakes;

	mspBackgroundClass->OnInitialise();

}

void		BackgroundUpdate( float fDelta )
{
	mspBackgroundClass->OnUpdate( fDelta );

}

void		BackgroundDisplay( float fIntensity )
{
	mspBackgroundClass->OnDisplay( fIntensity );

}

void		BackgroundDisplayUpperLayer( void )
{
	mspBackgroundClass->OnDisplayUpperLayer();
}


void		BackgroundShutdown( void )
{
	if ( mspBackgroundClass )
	{
		mspBackgroundClass->OnShutdown();

		delete mspBackgroundClass;
		mspBackgroundClass = NULL;
	}
}

