#ifndef DISPLAY_BACKGROUND_H
#define DISPLAY_BACKGROUND_H


class BackgroundBase
{
public:
	virtual ~BackgroundBase() 
	{

	}

	virtual void	OnInitialise( void ) = 0;

	virtual void	OnUpdate( float fDelta ) = 0;
	
	virtual void	OnDisplay( float fGlobalAlpha ) = 0;

	virtual void	OnDisplayUpperLayer( void ) {};

	virtual void	OnShutdown( void ) = 0;
};



extern void		BackgroundInitialise( void );

extern void		BackgroundUpdate( float fDelta );

extern void		BackgroundDisplay( float fGlobalAlpha );

extern void		BackgroundDisplayUpperLayer( void );

extern void		BackgroundShutdown( void );



#endif