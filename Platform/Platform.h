#ifndef GAMECOMMON_PLATFORM_H
#define GAMECOMMON_PLATFORM_H

#ifdef __cplusplus
extern "C"
{
#endif


//----------------------------------------------------------------------------------------------------
//  Platform interface
//----------------------------------------------------------------------------------------------------

extern void				PlatformInit( const char* szWindowName, BOOL bAllowResize, BOOL bAllowSleep );

extern BOOL				PlatformUpdateFrame( void );

extern float			PlatformGetFrameDelta( void );

//----------------------------------------------------------------------------------------------------
//  Platform SaveData
//----------------------------------------------------------------------------------------------------

extern int				PlatformSaveDataFileOpen( int slot, BOOL bWrite );
extern int				PlatformSaveDataFileWrite( unsigned char* pucData, int nMemSize );
extern int				PlatformSaveDataFileRead( unsigned char* pucData, int nMemSize );
extern int				PlatformSaveDataFileClose( void );


//----------------------------------------------------------------------------------------------------
//  Platform Ads
//----------------------------------------------------------------------------------------------------

extern void				PlatformAdsInitialise( const char* szAdSystemAppKey );
extern void				PlatformAdDisplay( BOOL bShow );

//----------------------------------------------------------------------------------------------------
//  Platform Analytics
//----------------------------------------------------------------------------------------------------

extern void				PlatformAnalyticsInit( const char* szAnalyticsKey );
extern void				PlatformLogEvent( const char* szEventString, const char* szKey, int nValue );

//----------------------------------------------------------------------------------------------------
//  Platform Commerce
//----------------------------------------------------------------------------------------------------
extern void				PlatformCommerceInit( const char* szCommerceKey );
extern void				PlatformCommercePurchase( const char* szProductID );
extern void				PlatformCommerceCheckPurchaseState( const char* szProductID );
extern BOOL				PlatformCommerceIsBusy( void );
extern BOOL				PlatformCommerceDidError( void );
extern const char*		PlatformCommerceGetLastErrorString( void );
extern void 			PlatformCommerceReset( void );
extern void 			PlatformCommerceFree( void );
extern void 			PlatformCommerceCancel( void );


//----------------------------------------------------------------------------------------------------
//  Platform Profile
//----------------------------------------------------------------------------------------------------
extern void				PlatformProfileMark( const char* szName );
extern void				PlatformProfileStart( const char* szName );
extern void				PlatformProfileStop( void );
extern void				PlatformProfileRender( void );
extern void				PlatformProfileTextMark( const char* szName );
extern void				PlatformProfileTextEnd( void );

//----------------------------------------------------------------------------------------------------
//  Platform Keyboard
//----------------------------------------------------------------------------------------------------
enum   // sent as nResponseCode in KeyboardMessageHandler callback
{
	KEYBOARD_ON_PRESS_ENTER = 1,
	KEYBOARD_ON_PRESS_TAB,
};

typedef	int		(*PlatformKeyboardMessageHandler)( int nResponseCode, const char* szInputText );
typedef	int		(*PlatformKeyboardSpecialKeyPressHandler)( int nKeyCode );
typedef	int		(*PlatformControlZoomHandler)( float fZoomAmount );

extern BOOL				PlatformHasKeyboard( void );
extern void				PlatformKeyboardActivate( int mode, const char* szStartText, const char* szPrompt );
extern void				PlatformKeyboardDeactivate( void );
extern BOOL				PlatformKeyboardOSKIsActive( void );
extern void				PlatformKeyboardRegisterHandler( PlatformKeyboardMessageHandler fnHandler );
extern const char*		PlatformKeyboardGetInputString( BOOL bIncludeCursor );
extern void				PlatformKeyboardSetInputString( const char* szNewInputString );
extern void				PlatformKeyboardRegisterSpecialKeyUpHandler( PlatformKeyboardSpecialKeyPressHandler fnHandler ); // For detecting keyup events for things like F-keys etc
extern void				PlatformKeyboardRegisterZoomControlHandler( PlatformControlZoomHandler fnHandler ); // For reacting to mousewheel

//-------------------------------------------------------------------------------------------------------
// Platform Misc
//-------------------------------------------------------------------------------------------------------

extern void				PlatformSetMouseOverCursor( BOOL );
extern void				PlatformGetCurrentCursorPosition( int* pnX, int* pnY );
extern void				PlatformSetCurrentCursorPosition( int X, int Y );

extern void				PlatformDebugText( const char* szText );
	
extern void				PlatformVibrate( int Mode );

extern void				PlatformResizeWindow( BOOL bFullScreen, int nWindowWidth, int nWindowHeight );
extern void				PlatformMoveWindow( int nLeft, int nTop );

extern void				PlatformGetFullscreenSize( int* pnWidth, int* pnHeight );
extern void*			PlatformGetWindowHandle( void );
extern void*			PlatformGetInstanceHandle( void );
extern const char*		PlatformGetStartupCommandLine( void );

extern BOOL				PlatformWindowIsFocused( void );

extern void				PlatformGetMACaddress( unsigned char* szOutBuffer );
extern void				PlatformGetGUID( char* szGUIDOut );		// Generates GUID, out buffer should be 256

extern void				PlatformRequestApplicationShutdown( void );

// set directly (used by Atractor)
extern void		PlatformSetWindow( int handle );

#ifdef __cplusplus
}
#endif


#endif


