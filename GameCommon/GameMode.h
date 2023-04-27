#ifndef GAME_MODE_H
#define GAME_MODE_H


#ifdef __cplusplus

// Modules are added to GameModes and have all the usual init, update, render and shutdown bits
class Module
{
public:
	Module() 
	{
		mpNext = NULL;
	}

	virtual ~Module();

	// InitModule is called when the module is added to a gamemode
	virtual void	OnInitModule( void ) = 0;
	virtual void	OnUpdate( float fDelta ) = 0;
	virtual void	OnRender( int nRenderPass ) {}
	virtual void	OnShutdown( void ) = 0;

	virtual void	OnReleaseGraphicsForDeviceReset( void ) {}
	virtual void	OnInitGraphicsPostDeviceReset( void ) {}

	void	InitModule( void );
	void	Update( float fDelta );
	void	Render( int nRenderPass );
	void	Shutdown( void );

	void	ReleaseGraphicsForDeviceReset( void );
	void	InitGraphicsPostDeviceReset( void );

	Module*			GetNext( void ) { return( mpNext ); }
	void			SetNext( Module* pNext ) { mpNext = pNext; }

	void	AddSubModule( Module* pModule );

private:	  
	Module*			mpNext;

	Module*			mpSubModulesList;
};

class GameMode
{
public:
	GameMode()
	{
		mpModulesList = NULL;
	}

	virtual		~GameMode() {}

	virtual void	OnInitGameMode( void ) = 0;
	virtual void	OnUpdate( float fDelta ) = 0;
	virtual void	OnRender( void ) = 0;
	virtual void	OnShutdown( void ) = 0;

	virtual void	OnReleaseGraphicsForDeviceReset( void ) {}
	virtual void	OnInitGraphicsPostDeviceReset( void ) {}

	virtual BOOL	OnPress( int X, int Y ) { return( FALSE ); }
	virtual BOOL	OnRelease( int X, int Y ) { return( FALSE ); }

	void	AddModule( Module* pModule );

	void	InitGameMode( void );
	void	Update( float fDelta );
	void	Render( void );
	void	Shutdown( void );
	void	ReleaseGraphicsForDeviceReset( void );
	void	InitGraphicsPostDeviceReset( void );

	void	RenderGameModeModules( int nRenderPass );

private:
	Module*			mpModulesList;
};



#endif


//----------------------------- Coz we all love C interfaces
#ifdef __cplusplus
extern "C"
{
#endif
	
enum
{
	GAMEMODE_FRONTEND,
	GAMEMODE_MAINGAME,
	GAMEMODE_MAINDRIVING,		// These need moving out of common i guess.. :)
	GAMEMODE_FLOCKTEST,
	GAMEMODE_PREGAME,
	GAMEMODE_BATTLE,
	GAMEMODE_CAMPAIGN,
};

extern void		GameModeInit( GameMode* pInitialMode );
extern void		GameModeChange( int nNewGameMode );

extern void		GameModeSet( GameMode* pGameMode );

extern void		GameModeUpdate( float fDelta );
extern void		GameModeRender( uint32 ulBackgroundClearColARGB );

extern void		GameModeShutdown( void );

extern void		GameModeShutdownSystem( void );			// Kill all the stuff that was created from static initialisers
	
extern BOOL		GameModeOnPress( int X, int Y );
extern BOOL		GameModeOnRelease( int X, int Y );

extern GameMode*		GameModeGetCurrent( void );

#ifdef __cplusplus
}
#endif







#endif