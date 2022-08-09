
#include "StandardDef.h"
#include "Interface.h"

#include "../UI/UI.h"
#include "../RenderUtil/Spinny.h"

#include "UserLoginClient.h"
#include "StandardClientLoginInterface.h"

enum
{
	SCLI_LOGINSTAGE_INITIAL_PAGE,
	SCLI_LOGINSTAGE_EXISTING_USER,
	SCLI_LOGINSTAGE_CREATE_NEW_USER,
	SCLI_LOGINSTAGE_LOGGING_IN,
	SCLI_LOGINSTAGE_ERROR,
};

enum
{
	SCLIBUTTON_CONNECT,
	SCLIBUTTON_CREATE_ACCOUNT,
	SCLIBUTTON_USE_EXISTING_ACCOUNT,
	SCLIBUTTON_BACK,
};

BYTE	mSCLIULSBlowfishClientKey[24] = { 0xD0, 0x41, 0x72, 0xA2, 0x17, 0x98,0x3B, 0x25,0xD1, 0xF6, 0xA5, 0x4B, 0x1C, 0x01, 0xE3, 0xF1, 0x19, 0x33,0x97,0xBC,0x23, 0x6C, 0x3D, 0xE4 };

char	mszStandardLoginInterfaceUserLoginServiceURL[128] = "";
char	mszStandardLoginInterfaceVersionString[32] = "";

char	mszStandardLoginInterfaceUserLoginEmail[128] = "";
char	mszStandardLoginInterfaceUserLoginPassword[64] = "";

int		mnStandardLoginInterfaceLogInProcessStage = SCLI_LOGINSTAGE_EXISTING_USER;

UserDetailsModifiedCallback		mpfnUserDetailsModifiedCallback = NULL;

BOOL		mbSCLIHasCompleted = FALSE;
BOOL		mbSCLIHasErrored = FALSE;
BOOL		mbSCLILogInProcessActive = FALSE;

int			mhSCLIUserEmailTextbox = NOTFOUND;
int			mhSCLIUserPasswordTextbox = NOTFOUND;

void	StandardClientLoginInterfaceNewUserDialogCallback( int nResponseCode )
{
	switch ( nResponseCode )
	{
	case 0:		// Cancelled
		mnStandardLoginInterfaceLogInProcessStage = SCLI_LOGINSTAGE_CREATE_NEW_USER;
		break;
	default:
		break;
	}
}

void	StandardClientLoginInterfaceSetUserDetailsCallback( UserDetailsModifiedCallback fnUserDetailsModifiedCallback )
{
	mpfnUserDetailsModifiedCallback = fnUserDetailsModifiedCallback;
}


void	StandardClientLoginInterfaceSetLoginDetails( const char* szLoginServiceURL, const char* szLoginVersionString )
{
	strcpy( mszStandardLoginInterfaceUserLoginServiceURL, szLoginServiceURL );
	strcpy( mszStandardLoginInterfaceVersionString, szLoginVersionString );
}


void	StandardClientLoginBeginLoginProcess( const char* szLoginEmail, const char* szLoginPassword )
{
	UserLoginClientLogin( mszStandardLoginInterfaceUserLoginServiceURL, szLoginEmail, szLoginPassword, mszStandardLoginInterfaceVersionString );

	mnStandardLoginInterfaceLogInProcessStage = SCLI_LOGINSTAGE_LOGGING_IN;

}


void		StandardLoginClientPageInitial(  int X, int Y, int W, int H )
{
int		nButtonX, nButtonY;
int		nButtonW = 400;
int		nButtonH = 90;

	nButtonX = (W - nButtonW) / 2;
	nButtonY = Y + (H / 2) - (nButtonH + 10 );
	UIButtonDraw( UIRESERVEDBUTTONID_STANDARDLOGININTERFACE, nButtonX, nButtonY, nButtonW, nButtonH, "CREATE ACCOUNT", 0, SCLIBUTTON_CREATE_ACCOUNT );	
	nButtonY += nButtonH + 20;
	UIButtonDraw( UIRESERVEDBUTTONID_STANDARDLOGININTERFACE, nButtonX, nButtonY, nButtonW, nButtonH, "USE EXISTING ACCOUNT", 0, SCLIBUTTON_USE_EXISTING_ACCOUNT );	
}


void		StandardLoginClientPageNewUser(  int X, int Y, int W, int H )
{
	UserLoginClientNewUserDialogRender( X + 10, Y + 10, W - 20, H - 20 );

	if ( UserLoginClientNewUserCreated() == TRUE )
	{
	const char*		szLoginEmail = UserLoginClientNewUserGetEmail();
	const char*		szLoginPassword = UserLoginClientNewUserGetPassword();

		strcpy( mszStandardLoginInterfaceUserLoginEmail, szLoginEmail );
		strcpy( mszStandardLoginInterfaceUserLoginPassword, szLoginPassword );

		if ( mpfnUserDetailsModifiedCallback )
		{
			mpfnUserDetailsModifiedCallback( szLoginEmail, szLoginPassword );
		}

		StandardClientLoginBeginLoginProcess( szLoginEmail, szLoginPassword );
	}
}


const char*		StandardLoginClientGetShortStatusString( void )
{
	switch ( UserLoginClientGetLoginState() )
	{
	case LOGIN_CLOSED:
		return( "Game closed" );
		break;
	case LOGIN_ERROR:
		return( "Login Error" );
		break;
	case LOGIN_REFUSED_INCORRECT_PASSWORD:
		return( "Login Error : Password did not match" );
		break;		
	case LOGIN_REFUSED_UNRECOGNISED_USERNAME:
		return( "Login Error : User login email not recognised" );
		break;	
	case LOGIN_REFUSED_BANNED:
		return( "Login failed. You cannot currently access the game" );
		break;			
	case LOGIN_SUCCESS:
		return( "Connected" );
		break;
	case LOGIN_IN_PROGRESS:
		switch( UserLoginClientGetLoginPhase() )
		{
		case 0:
			return( "Connecting..." );
			break;
		case 1:
			return( "Authenticating..." );
			break;
		default:
			return( "Logging in.." );
			break;
		}
		break;
	case LOGIN_IDLE:
	default:
		return( "Connect State Error" );
		break;
	}

}

void		StandardLoginClientPageLoggingIn(  int X, int Y, int W, int H )
{
BOOL	bShowCancelButton = FALSE;
int		nButtonX;
int		nButtonY;
int		nButtonW = 400;
int		nButtonH = 90;
float	fGlobalAlpha = 1.0f;
int		nTextX = X + 100;
int		nTextW = W - 200;
const char*		pcClosedText = UserLoginClientGetLastResponseText();
int		nLastResponseCode = UserLoginGetLastResponseCode();

	InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );
	nButtonX = X + ( (W - nButtonW) / 2 );
	nButtonY = Y + ( (H / 2 ) - (int)(nButtonH * 1.5f) );
	switch ( UserLoginClientGetLoginState() )
	{
	case LOGIN_CLOSED:
	case LOGIN_ERROR:
		if ( ( nLastResponseCode == 420 ) ||
			 ( nLastResponseCode == 421 ) )
		{
			InterfaceTextCenter( 1, X, X + W, nButtonY, "Game is Closed", 0xD0F0F0E8, 1 );
		}
		else
		{
			InterfaceTextCenter( 1, X, X + W, nButtonY, "Login Error", 0xD0F0F0E8, 1 );
		}

		switch( nLastResponseCode )
		{
		case 0:
		default:
			InterfaceTextBox( 1, nTextX, nButtonY + 40, "There has been a problem connecting to the game servers, please try later. If this problem persists, check for updates at http://atractor.net.", 0xD0F0F0E8, 0, nTextW, FALSE );
			break;
		case 400:		// NOTACTIVE
			InterfaceTextBox( 1, nTextX, nButtonY + 40, "Your account has not yet been authorised - you will receive an email when this has happened.", 0xD0F0F0E8, 0, nTextW, FALSE );
			break;
		case 403:		// INVALID
			InterfaceTextBox( 1, nTextX, nButtonY + 40, "Your account does not have permission to access this title.", 0xD0F0F0E8, 0, nTextW, FALSE );
			break;
		case 401:		// PWMATCH
			InterfaceTextBox( 1, nTextX, nButtonY + 40, "Your password did not match the one we have on record. You can check your details or retrieve a forgotten password through the alpha forums at http://atractor.net.", 0xD0F0F0E8, 0, nTextW, FALSE );
			break;
		case 404:		// NAME404 +
			InterfaceTextBox( 1, nTextX, nButtonY + 40, "Your email address was not found in our records. Please make sure you're registered on the alpha forums at http://atractor.net.", 0xD0F0F0E8, 0, nTextW, FALSE );
			break;
		case 405:		// VERSION
			InterfaceTextBox( 1, nTextX, nButtonY + 40, "Your version of the game is no longer supported. Please check for updates at http://atractor.net.", 0xD0F0F0E8, 0, nTextW, FALSE );
			break;
		case 402:		// Generic
			if ( ( pcClosedText ) &&
				 ( pcClosedText[0] != 0 ) )
			{
				InterfaceTextBox( 1, nTextX, nButtonY + 40, pcClosedText, 0xD0F0F0E8, 0, nTextW, FALSE );
			}
			else
			{
				InterfaceTextBox( 1, nTextX, nButtonY + 40, "There has been a problem connecting to the game servers, please try later. If this problem persists, check for updates at http://atractor.net.", 0xD0F0F0E8, 0, nTextW, FALSE );
			}
			break;
		case 420:		// Closed
		case 421:		// Closed with pub
			if ( ( pcClosedText ) &&
				 ( pcClosedText[0] != 0 ) )
			{
				InterfaceTextBox( 1, nTextX, nButtonY + 40, pcClosedText, 0xF0F0F0F0, 0, nTextW, FALSE );
			}
			else
			{
				InterfaceTextBox( 1, nTextX, nButtonY + 40, "The game is currently closed. Please try later. Updates will be found at http://atractor.net.", 0xD0F0F0E8, 0, nTextW, FALSE );
			}
			break;
		}
		bShowCancelButton = TRUE;
		break;
	case LOGIN_REFUSED_INCORRECT_PASSWORD:
		InterfaceTextCenter( 1, X, X + W, nButtonY, "Password did not match", 0xD0F0F0E8, 1 );
		bShowCancelButton = TRUE;
		break;		
	case LOGIN_REFUSED_UNRECOGNISED_USERNAME:
		InterfaceTextCenter( 1, X, X + W, nButtonY, "Login email not recognised", 0xD0F0F0E8, 1 );
		bShowCancelButton = TRUE;
		break;	
	case LOGIN_REFUSED_BANNED:
		InterfaceTextCenter( 1, X, X + W, nButtonY, "You were not allowed to access this game", 0xD0F0F0E8, 1 );
		bShowCancelButton = TRUE;
		break;			
	case LOGIN_SUCCESS:
		InterfaceTextCenter( 1, X, X + W, nButtonY, "Connected", 0xD0F0F0E8, 1 );
		InterfaceSetFontFlags( 0 );
//		InterfaceTextCenter( 1, X, X + W, nButtonY + 25, "Fetching game data", 0xD0D0D0D0, 0 );
		SpinnyDraw( X + (W/2) - 30, nButtonY + 70, 60, 60, fGlobalAlpha );
		break;
	case LOGIN_IN_PROGRESS:
		InterfaceTextCenter( 1, X, X + W, nButtonY, "Logging in..", 0xD0F0F0E8, 1 );
		InterfaceSetFontFlags( 0 );
		switch( UserLoginClientGetLoginPhase() )
		{
		case 0:
			InterfaceTextCenter( 1, X, X + W, nButtonY + 25, "Connecting", 0xD0F0F0E8, 0 );
			break;
		case 1:
			InterfaceTextCenter( 1, X, X + W, nButtonY + 25, "Authenticating", 0xD0F0F0E8, 0 );
			break;
		}
		SpinnyDraw( X + (W/2) - 30, nButtonY + 70, 60, 60, fGlobalAlpha );
		break;
	case LOGIN_IDLE:
	default:
		InterfaceTextCenter( 1, X, X + W, nButtonY, "Connect state error", 0xD0F0F0E8, 1 );
		bShowCancelButton = TRUE;
		break;
	}
	InterfaceSetFontFlags( 0 );

	if ( bShowCancelButton )
	{
		// todo ..

//		ScreenMode::AddStandardBackButton( fGlobalAlpha, X, Y, W, H, UIRESERVEDBUTTONID_STANDARDLOGININTERFACE, SCLIBUTTON_BACK );
	}

}

void		StandardLoginClientPageExistingUser(  int X, int Y, int W, int H )
{
int		nIdealWidth = 500;
int		nColumnX = X + 220;
int		nButtonX, nButtonY;
int		nButtonW;
int		nButtonH = 90;
int		nHeaderW = (int)( W * 0.8f );
int		nHeaderX = X + (int)( W * 0.1f );

	if ( W < nIdealWidth )
	{
		nColumnX = X + 120;
		nButtonW = W - 140;
	}
	else
	{
		nButtonW = W - 270;
	}

	InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );
	InterfaceTextBox( 1, nHeaderX, Y, "To sign in, use your account details registered on the forums at http://gamesformay.com", 0xd0d0d0d0, 0, nHeaderW, TRUE );
	InterfaceSetFontFlags( 0 );

	nButtonX = nColumnX + 10;
	nButtonH = H / 6;
	nButtonY = Y + ( ( (H - (H/4)) / 2 ) - (int)(nButtonH * 1.5f) );

	InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );
	InterfaceTextRight(1, nColumnX, nButtonY + ((nButtonH-22)/2), "Login Email:", 0xd0d0d0d0, 1 );
	InterfaceSetFontFlags( 0 );

	if ( nButtonW > 400 ) nButtonW = 400;

	UITextBoxRender( mhSCLIUserEmailTextbox, nButtonX, nButtonY, nButtonW, nButtonH );
	nButtonY += nButtonH + 10;
	InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );
	InterfaceTextRight(1, nColumnX, nButtonY + ((nButtonH-22)/2), "Password:", 0xd0d0d0d0, 1 );
	InterfaceSetFontFlags( 0 );

	if ( nButtonW > 300 ) nButtonW = 300;

	UITextBoxRender( mhSCLIUserPasswordTextbox, nButtonX, nButtonY, nButtonW, nButtonH );

	nButtonH = H/4;
	nButtonY = Y + H - (nButtonH + 40);
	nButtonX = (W-nButtonW)/2;
	UIButtonDraw( UIRESERVEDBUTTONID_STANDARDLOGININTERFACE, nButtonX, nButtonY, nButtonW, nButtonH, "CONNECT", 0, SCLIBUTTON_CONNECT );

#ifdef SHOW_ENVIRONMENTS_BUTTON
	nButtonH = 50;
	nButtonW = 160;
	nButtonY = InterfaceGetHeight() - 60;
	nButtonX = InterfaceGetWidth() - 170;

	if ( mspServiceConfig == &msDevServiceConfig )
	{
		UIButtonDraw( UIRESERVEDBUTTONID_STANDARDLOGININTERFACE, nButtonX, nButtonY, nButtonW, nButtonH, "Env: Dev", 0, SCLIBUTTON_DEBUG_ENV );
	}
	else if ( mspServiceConfig == &msLocalServiceConfig )
	{
		UIButtonDraw( UIRESERVEDBUTTONID_STANDARDLOGININTERFACE, nButtonX, nButtonY, nButtonW, nButtonH, "Env: Local", 0, SCLIBUTTON_DEBUG_ENV );
	}
	else
	{
		UIButtonDraw( UIRESERVEDBUTTONID_STANDARDLOGININTERFACE, nButtonX, nButtonY, nButtonW, nButtonH, "Env: Live", 0, SCLIBUTTON_DEBUG_ENV );
	}
#endif
}


void		StandardClientLoginDisplay( int X, int Y, int W, int H )
{
	switch( mnStandardLoginInterfaceLogInProcessStage )
	{
	case SCLI_LOGINSTAGE_INITIAL_PAGE:
		StandardLoginClientPageInitial( X, Y, W, H );
		break;
	case SCLI_LOGINSTAGE_CREATE_NEW_USER:
		StandardLoginClientPageNewUser( X, Y, W, H );
		break;
	case SCLI_LOGINSTAGE_EXISTING_USER:
		StandardLoginClientPageExistingUser( X, Y, W, H );
		break;
	case SCLI_LOGINSTAGE_LOGGING_IN:
		StandardLoginClientPageLoggingIn( X, Y, W, H );
		break;
	}

	if ( mbSCLILogInProcessActive == TRUE )
	{
		if ( UserLoginClientGetLoginState() != LOGIN_IN_PROGRESS )
		{
			mbSCLIHasCompleted = TRUE;
		}
	}
}

BOOL		StandardClientLoginIsComplete( void )
{
	return( mbSCLIHasCompleted );
}

void		StandardClientLoginReset( void )
{
	if ( mbSCLILogInProcessActive == TRUE )
	{


	}
	mbSCLIHasCompleted = FALSE;
	mbSCLILogInProcessActive = FALSE;
	mbSCLIHasErrored = FALSE;
}


void		StandardLoginClientResetLoginProcess( BOOL bLoginDetailsAlreadyValidated )
{
	mbSCLILogInProcessActive = TRUE;
	mbSCLIHasErrored = FALSE;
	
#ifdef ALLOW_CREATE_NEW_USER
	mnStandardLoginInterfaceLogInProcessStage = SCLI_LOGINSTAGE_INITIAL_PAGE;
#else
	mnStandardLoginInterfaceLogInProcessStage = SCLI_LOGINSTAGE_EXISTING_USER;
#endif
}

void	StandardLoginClientUIButtonPressHandler( int nButtonID, ulong ulParam )
{
	switch( ulParam )
	{
	case SCLIBUTTON_CONNECT:
		{
			UITextBoxEndEdit( mhSCLIUserEmailTextbox );
			UITextBoxEndEdit( mhSCLIUserPasswordTextbox );

			const char*		szLoginEmail = UITextBoxGetText( mhSCLIUserEmailTextbox );
			const char*		szLoginPassword = UITextBoxGetText( mhSCLIUserPasswordTextbox );

			if ( mpfnUserDetailsModifiedCallback )
			{
				mpfnUserDetailsModifiedCallback( szLoginEmail, szLoginPassword );
			}	

			StandardClientLoginBeginLoginProcess( szLoginEmail, szLoginPassword );

			mnStandardLoginInterfaceLogInProcessStage = SCLI_LOGINSTAGE_LOGGING_IN;
		}
		break;
	case SCLIBUTTON_CREATE_ACCOUNT:
		UserLoginClientNewUserDialogInit( mszStandardLoginInterfaceUserLoginServiceURL, StandardClientLoginInterfaceNewUserDialogCallback );
		mnStandardLoginInterfaceLogInProcessStage = SCLI_LOGINSTAGE_CREATE_NEW_USER;
		break;
	case SCLIBUTTON_USE_EXISTING_ACCOUNT:
		mnStandardLoginInterfaceLogInProcessStage = SCLI_LOGINSTAGE_EXISTING_USER;
		break;
	case SCLIBUTTON_BACK:
		StandardLoginClientResetLoginProcess( FALSE );
		break;
	}
}


void	StandardClientLoginBegin( const char* szLoginEmail, const char* szLoginPassword )
{
	UserLoginClientSetKey( mSCLIULSBlowfishClientKey, 24 );

	if ( szLoginEmail )
	{
		strcpy( mszStandardLoginInterfaceUserLoginEmail, szLoginEmail );
	}

	if ( szLoginPassword )
	{
		strcpy( mszStandardLoginInterfaceUserLoginPassword, szLoginPassword );
	}

	if ( mbSCLILogInProcessActive == FALSE )
	{
		UIRegisterButtonPressHandler( UIRESERVEDBUTTONID_STANDARDLOGININTERFACE, StandardLoginClientUIButtonPressHandler );

		mhSCLIUserEmailTextbox = UITextBoxCreate( 0, mszStandardLoginInterfaceUserLoginEmail, 200 );
		mhSCLIUserPasswordTextbox = UITextBoxCreate( 1, mszStandardLoginInterfaceUserLoginPassword, 24 );

		StandardLoginClientResetLoginProcess( FALSE );

		if ( mszStandardLoginInterfaceUserLoginEmail[0] != 0 )
		{
			StandardClientLoginBeginLoginProcess( mszStandardLoginInterfaceUserLoginEmail, mszStandardLoginInterfaceUserLoginPassword );
		}

	}
}
