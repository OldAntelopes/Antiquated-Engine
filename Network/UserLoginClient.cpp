
// ---- todo - this file should really be called UserLoginClient has it handles the communication
//   between client and UserLoginService
//  (tho also contains some useful more generic functions like 'WebServerDecrypt')

#include <stdio.h>
#include "StandardDef.h"
#include "Interface.h"

#include "../RenderUtil/Spinny.h"
#include "../UI/UI.h"
#include "../Util/Blowfish.h"
#include "../Util/cJSON.h"

#include "../HTTP/HTTP.h"

#include "Steam/steam_api.h"
#include "UserLoginClient.h"

#ifdef SERVER
#define NODISPLAY
#endif

#ifdef _DEBUG
//#define	LOGIN_WITH_DEV_FLAG_SET
#endif

enum eNewUserLoginStage
{ 
	NEWUSER_ENTER_DETAILS,
	NEWUSER_SUBMITTING,
	NEWUSER_USERNAME_TAKEN,
	NEWUSER_EMAIL_USED,
	NEWUSER_CREATE_ERROR,
	NEWUSER_VALID_RESPONSE,
};

BLOWFISH_KEY		mxBlowfishUserLoginClientKey;
char				mszUserLoginClientSessionID[32] = "";

char				mszLoginServerBaseURL[256] = "";
char*				mpcSessionKey = NULL;
char				mszLoginClientUserName[64] = "";
char				mszLoginTokenStore[64] = "";
u64					mullUserUID = 0;
int					mnMasterGameState = 0;
char*				mpcTicketResponse = NULL;

char*				msszUserLoginLastResponseText = NULL;

eUserLoginClientState		msUserLoginClientLoginState = LOGIN_IDLE;
int							msUserLoginLastResponseCode = 0;
eNewUserLoginStage			mnNewUserLoginStage = NEWUSER_ENTER_DETAILS;
int							msUserLoginPhase = 0;
BOOL						msbWaitingForSteamAppTicket = FALSE;

void		UserLoginClientSetKey( BYTE* pbUserKey, int nKeyLen )
{
	BlowfishSetKey( pbUserKey, nKeyLen, &mxBlowfishUserLoginClientKey );

}

void		UserLoginClientReset( void )
{
	SAFE_FREE( mpcSessionKey );
	SAFE_FREE( mpcTicketResponse );
	msUserLoginClientLoginState = LOGIN_IDLE;
	msUserLoginLastResponseCode = 0;
	mnNewUserLoginStage = NEWUSER_ENTER_DETAILS;
	msUserLoginPhase = 0;
	mullUserUID = 0;
	mszLoginClientUserName[0] = 0;
	mszUserLoginClientSessionID[0] = 0;

}

void		UserLoginClientShutdown( void )
{
	SAFE_FREE( mpcSessionKey );
	SAFE_FREE( mpcTicketResponse );
}

const char*		UserLoginClientGetUserName( void )
{
	if ( mszLoginClientUserName[0] == 0 )
	{
		return( NULL );
	}
	return( mszLoginClientUserName );
}

u64				UserLoginClientGetUserUID( void )
{
	return( mullUserUID );
}

void			UserLoginClientDecrypt( BYTE* pbMem, int nMemLen )
{
int		nNumBlocks;
BYTE*	pbMemBuffIn;
BYTE*	pbMemBuffOut;
int		nBlockLoop;

	nNumBlocks = ((nMemLen-1) / 8) + 1;

	pbMemBuffIn = (BYTE*)SystemMalloc( (nNumBlocks*8) + 1 );
	memset( pbMemBuffIn, 0, (nNumBlocks*8) + 1 );
	pbMemBuffOut = (BYTE*)SystemMalloc( (nNumBlocks*8) + 1 );

	memcpy( pbMemBuffIn, pbMem, nMemLen );

	for ( nBlockLoop = 0; nBlockLoop < nNumBlocks; nBlockLoop++ )
	{
		BlowfishDecrypt( pbMemBuffIn + (nBlockLoop*8), pbMemBuffOut + (nBlockLoop*8), &mxBlowfishUserLoginClientKey );
	}
	memcpy( pbMem, pbMemBuffOut, nMemLen );

	free( pbMemBuffIn );
	free( pbMemBuffOut );
}

int			UserLoginClientEncrypt( BYTE* pbMem, int nMemLen )
{
int		nNumBlocks;
BYTE*	pbMemBuffIn;
BYTE*	pbMemBuffOut;
int		nBlockLoop;

	nNumBlocks = ((nMemLen-1) / 8) + 1;
	nMemLen = nNumBlocks * 8;

	pbMemBuffIn = (BYTE*)SystemMalloc( nMemLen + 8 );
	pbMemBuffOut = (BYTE*)SystemMalloc( nMemLen + 8 );
	memset( pbMemBuffIn, 0, nMemLen + 8 );
	memset( pbMemBuffOut, 0, nMemLen + 8 );
	memcpy( pbMemBuffIn, pbMem, nMemLen );

	for ( nBlockLoop = 0; nBlockLoop < nNumBlocks; nBlockLoop++ )
	{
		BlowfishEncrypt( (uchar*)pbMemBuffIn + (nBlockLoop*8), (uchar*)pbMemBuffOut + (nBlockLoop*8), &mxBlowfishUserLoginClientKey );
	}
	memcpy( pbMem, pbMemBuffOut, nMemLen + 8 );
	free( pbMemBuffIn );
	free( pbMemBuffOut );
	return( nMemLen );
}

//---------------------------------------------------------------------------------

const char*		UserLoginClientGetSessionKey( void )
{
	return( mpcSessionKey );
}

const char*		UserLoginClientGetLastResponseText( void )
{
	return( msszUserLoginLastResponseText );
}


void		UserLoginClientGetSteamSessionResponseHandler( int nResponseCode, unsigned char* szResponseBody, int nResponseLen, void* vpParam )
{
	if ( nResponseCode == 200 )
	{
	cJSON*	pJSONRoot;
	cJSON*	pItem;
//	u64		ulMyUserID;
	char*	pcMySessionKey;
	char*	pcTicket;

		pJSONRoot = cJSON_Parse( (char*)szResponseBody );

		// (TEMP?) Set my userUID to my SteamID
		mullUserUID = SteamUser()->GetSteamID().ConvertToUint64();

		pItem = cJSON_GetObjectItem( pJSONRoot, "SESSION" );

		pcMySessionKey = pItem->valuestring;
//		printf( "Got my sessionID: %s\n", pcMySessionKey );
		SAFE_FREE( mpcSessionKey );
		mpcSessionKey = (char*)( SystemMalloc( strlen(pcMySessionKey) + 1 ) );
		strcpy( mpcSessionKey, pcMySessionKey );

		// TEMP! Contents of ticket response should be encrypted
		// (and possibly not just sent here as its not exactly scalable)
		pItem = cJSON_GetObjectItem( pJSONRoot, "TICKETSV1" );
		if ( pItem )
		{
			SAFE_FREE( mpcTicketResponse )
			pcTicket = pItem->valuestring;
			if ( pcTicket )
			{
				mpcTicketResponse = (char*)( SystemMalloc( strlen(pcTicket) + 1 ) );
				strcpy( mpcTicketResponse, pcTicket );
			}
		}

		msUserLoginClientLoginState = LOGIN_SUCCESS;

		cJSON_Delete( pJSONRoot );
	}
	else
	{

	}

}

void		UserLoginClientGetSessionResponseHandler( int nResponseCode, unsigned char* szResponseBody, int nResponseLen, void* vpParam )
{
	if ( nResponseCode == 200 )
	{
	cJSON*	pJSONRoot;
	cJSON*	pItem;
//	u64		ulMyUserID;
	char*	pcMySessionKey;
	char*	pcTicket;

		pJSONRoot = cJSON_Parse( (char*)szResponseBody );
		pItem = cJSON_GetObjectItem( pJSONRoot, "USERID" );

		mullUserUID = (u64)( pItem->valuedouble );

		pItem = cJSON_GetObjectItem( pJSONRoot, "SESSION" );

		pcMySessionKey = pItem->valuestring;
//		printf( "Got my sessionID: %s\n", pcMySessionKey );
		SAFE_FREE( mpcSessionKey );
		mpcSessionKey = (char*)( SystemMalloc( strlen(pcMySessionKey) + 1 ) );
		strcpy( mpcSessionKey, pcMySessionKey );

		// TEMP! Contents of ticket response should be encrypted
		// (and possibly not just sent here as its not exactly scalable)
		pItem = cJSON_GetObjectItem( pJSONRoot, "TICKETSV1" );
		if ( pItem )
		{
			SAFE_FREE( mpcTicketResponse )
			pcTicket = pItem->valuestring;
			if ( pcTicket )
			{
				mpcTicketResponse = (char*)( SystemMalloc( strlen(pcTicket) + 1 ) );
				strcpy( mpcTicketResponse, pcTicket );
			}
		}

		msUserLoginClientLoginState = LOGIN_SUCCESS;

		cJSON_Delete( pJSONRoot );
	}
	else
	{
		msUserLoginLastResponseCode = nResponseCode;

		switch( nResponseCode )
		{
		case 420:
			msUserLoginClientLoginState = LOGIN_CLOSED;
			break;
		case 421:
			msUserLoginClientLoginState = LOGIN_CLOSED;
			break;
		default:
			msUserLoginClientLoginState = LOGIN_ERROR;
			break;
		}	
	}
	
}


void		UserLoginClientLoginSendTokenResponse( const char* szLoginToken )
{
char	acBuff[256];
char	acLoginURL[256];
int		nContentLength;
int		nSalt1 = (rand() << 16) | ( rand() );
int		nSalt2 = (rand() << 16) | ( rand() );

	memset( acBuff, 0, 256 );
	// todo
	sprintf( acBuff, "%08xtok=%s%08x", nSalt1, szLoginToken, nSalt2 );
	nContentLength = strlen( acBuff ) + 1;

	// Encrypt rounds up to 8 byte blocks..
	nContentLength = UserLoginClientEncrypt( (BYTE*)acBuff, nContentLength  );

//	SysUserPrint( 0, "Sending login token: %s BodyLen: %d", szLoginToken, nContentLength );

	sprintf( acLoginURL, "%s/getsession?token=%s", mszLoginServerBaseURL, szLoginToken );
	HTTPPost( acLoginURL, (BYTE*)acBuff, nContentLength, UserLoginClientGetSessionResponseHandler, 0 );
	
}

int		UserLoginClientGetMasterGameState( void )
{
	return( mnMasterGameState );
}


class SteamLoginManager
{
public:
	void GetEncryptedAppTicket( const char* szLoginTicket );

private:
	void OnGetEncryptedAppTicket( EncryptedAppTicketResponse_t *pCallback, bool bIOFailure );
	CCallResult< SteamLoginManager, EncryptedAppTicketResponse_t > m_EncryptedAppTicketCallResult;
};

SteamLoginManager		msSteamLoginManager;

void SteamLoginManager::OnGetEncryptedAppTicket( EncryptedAppTicketResponse_t *pEncryptedAppTicketResponse, bool bIOFailure )
{
	msbWaitingForSteamAppTicket = FALSE;

	if ( bIOFailure )
	{
		msUserLoginLastResponseCode = -99;
		msUserLoginClientLoginState = LOGIN_ERROR;
		return;
	}

	if ( pEncryptedAppTicketResponse->m_eResult == k_EResultOK )
	{
	BYTE	abTicket[1024];
	uint32	nSizeOfTicket;		

		SteamUser()->GetEncryptedAppTicket( abTicket, sizeof( abTicket), &nSizeOfTicket );

		// normally at this point you transmit the encrypted ticket to the service that knows the decryption key..
		if ( msUserLoginPhase == 1 )
		{
		char	acLoginURL[256];

			sprintf( acLoginURL, "%s/getsteamsession?token=%s", mszLoginServerBaseURL, mszLoginTokenStore );
			HTTPPost( acLoginURL, (BYTE*)abTicket, nSizeOfTicket, UserLoginClientGetSteamSessionResponseHandler, 0 );
		}
		else
		{
			// TODO!! ???
			msUserLoginPhase = 2;
		}
	}
	else
	{
		// mmm.. what to do about this?? Wait a minute?
		if ( pEncryptedAppTicketResponse->m_eResult == k_EResultLimitExceeded )
		{
			msUserLoginLastResponseCode = -97;
			msUserLoginClientLoginState = LOGIN_ERROR;
		}
		else
		{
			msUserLoginLastResponseCode = -98;
			msUserLoginClientLoginState = LOGIN_ERROR;
		}
	}

}

typedef struct
{
	ulong		ulSalt1;
	char		szLoginToken[24];
	u64			ullSteamID;
	ulong		ulSalt2;
	char		szPersonaName[32];
	ulong		ulIDNum;
	ulong		ulAppID;

} ENCRYPTED_TICKET_DATA;

void		SteamLoginManager::GetEncryptedAppTicket( const char* szLoginToken )
{
int			nDataSize;
const char*		pcLocalPlayerName;
ENCRYPTED_TICKET_DATA		xTicketData;

	pcLocalPlayerName = SteamFriends()->GetPersonaName();

	nDataSize = sizeof( ENCRYPTED_TICKET_DATA );
	memset( &xTicketData, 0, nDataSize );
	strncpy( xTicketData.szLoginToken, szLoginToken, 24 );
	strncpy( xTicketData.szPersonaName, pcLocalPlayerName, 31 );
	xTicketData.ullSteamID = SteamUser()->GetSteamID().ConvertToUint64();
	xTicketData.ulAppID = 0;	// TODO 
	xTicketData.ulIDNum = 0x6dfa1387;
	xTicketData.ulSalt1 = (rand() << 16) | ( rand() );
	xTicketData.ulSalt2 = (rand() << 16) | ( rand() );

	msbWaitingForSteamAppTicket = TRUE;

	SteamAPICall_t hSteamAPICall = SteamUser()->RequestEncryptedAppTicket( (BYTE*)&xTicketData, nDataSize );
	m_EncryptedAppTicketCallResult.Set( hSteamAPICall, this, &SteamLoginManager::OnGetEncryptedAppTicket );

}



void		UserLoginClientRequestSteamLoginTicket( const char* szLoginToken )
{


	msSteamLoginManager.GetEncryptedAppTicket(szLoginToken);
}

void		UserLoginRemoveUnsupportedCharactersFromLoginName( char* pcUserName )
{
char*		pcRunner = pcUserName;

	// Keep it under 24 chars so can work with universal worlds..
	pcUserName[23] = 0;
			
	while( *pcRunner != 0 )
	{
		if ( *pcRunner == ' ' )
		{
			*pcRunner = '_';
		}
		else if ( ( *pcRunner == '[' ) ||
			 ( *pcRunner == ']' ) ||
			 ( *pcRunner == '(' ) ||
			 ( *pcRunner == ')' ) ||
			 ( *pcRunner == '~' ) ||
			 ( *pcRunner == '#' ) ||
			 ( *pcRunner == '@' ) ||
			 ( *pcRunner == '!' ) ||
			 ( *pcRunner == '*' ) ||
			 ( *pcRunner == '&' ) ||
			 ( *pcRunner == '\'' ) ||
			 ( *pcRunner == '\"' ) ||
			 ( *pcRunner == '.' ) ||
			 ( *pcRunner == ',' ) ||
			 ( *pcRunner < 32 ) ||
			 ( *pcRunner > 128 ) )
		{
			*pcRunner = '^';
		}
		pcRunner++;
	}
}

void		UserLoginClientSteamLoginResponseHandler( int nResponseCode, unsigned char* szResponseBody, int nResponseLen, void* vpParam )
{
	if ( nResponseCode == 200 )
	{
	cJSON*	pJSONRoot;
	const char*	pcLoginToken;

		msUserLoginPhase = 1;
		pJSONRoot = cJSON_Parse( (char*)szResponseBody );
		if ( pJSONRoot )
		{
			pcLoginToken = cJSON_GetObjectValueText( pJSONRoot, "LOGINTOKEN" );
			mnMasterGameState = cJSON_GetObjectValueInt( pJSONRoot, "MASTERGAMESTATE" );

			strcpy( mszLoginClientUserName, SteamFriends()->GetPersonaName() );
			UserLoginRemoveUnsupportedCharactersFromLoginName(mszLoginClientUserName);

			strncpy( mszLoginTokenStore, pcLoginToken, 32 );
			// theoretically, next step is to parse the logintoken and userid, generate an encrypted session key
			// then send that back to the server to confirm our identity as a valid client. 
			// (this is needed to avoid replay attacks, where someone could just duplicate the send of the encrypted login msg)
			UserLoginClientRequestSteamLoginTicket( pcLoginToken );

//			UserLoginClientLoginSendTokenResponse( pcLoginToken );

			cJSON_Delete( pJSONRoot );
		}
		else
		{
			msUserLoginLastResponseCode = nResponseCode;
			msUserLoginClientLoginState = LOGIN_ERROR;
		}
	}
	else
	{
		msUserLoginLastResponseCode = nResponseCode;
		msUserLoginClientLoginState = LOGIN_ERROR;
	}

}

void		UserLoginClientLoginResponseHandler( int nResponseCode, unsigned char* szResponseBody, int nResponseLen, void* vpParam )
{
	if ( nResponseCode == 200 )
	{
	cJSON*	pJSONRoot;
	u64		ulMyUserID;
	const char*	pcMyUserName;
	const char*	pcLoginToken;

		msUserLoginPhase = 1;
		pJSONRoot = cJSON_Parse( (char*)szResponseBody );
		if ( pJSONRoot )
		{
			ulMyUserID = cJSON_GetObjectValueU64( pJSONRoot, "USERID" );
			pcMyUserName = cJSON_GetObjectValueText( pJSONRoot, "USERNAME" );	
			strcpy( mszLoginClientUserName, pcMyUserName );
			pcLoginToken = cJSON_GetObjectValueText( pJSONRoot, "LOGINTOKEN" );
			mnMasterGameState = cJSON_GetObjectValueInt( pJSONRoot, "MASTERGAMESTATE" );

			// theoretically, next step is to parse the logintoken and userid, generate an encrypted session key
			// then send that back to the server to confirm our identity as a valid client. 
			// (this is needed to avoid replay attacks, where someone could just duplicate the send of the encrypted login msg)
			UserLoginClientLoginSendTokenResponse( pcLoginToken );

			cJSON_Delete( pJSONRoot );
		}
		else  // invalid json
		{
			// todo - error handling
			msUserLoginClientLoginState = LOGIN_ERROR;
		}
	}
	else
	{
		// todo - error handling
		msUserLoginLastResponseCode = nResponseCode;

		switch( nResponseCode )
		{
		default:
			msUserLoginClientLoginState = LOGIN_ERROR;
			break;
		case 401:
			msUserLoginClientLoginState = LOGIN_REFUSED_INCORRECT_PASSWORD;
			break;
		case 404:
			msUserLoginClientLoginState = LOGIN_REFUSED_UNRECOGNISED_USERNAME;
			break;
		case 420:
		case 421:
			msUserLoginClientLoginState = LOGIN_CLOSED;
			
			// If theres a body it will be a text msg to display on screen
			if ( ( nResponseLen > 0 ) && 
				 ( szResponseBody ) )
			{
				SAFE_FREE( msszUserLoginLastResponseText );
				msszUserLoginLastResponseText = (char*)( SystemMalloc( nResponseLen + 1 ) );
				msszUserLoginLastResponseText[nResponseLen] = 0;
				memcpy( msszUserLoginLastResponseText, szResponseBody, nResponseLen );
			}
			break;
		}
	}

}

int		UserLoginGetLastResponseCode( void )
{
	return( msUserLoginLastResponseCode );
}


void		UserLoginClientSteamLogin( const char* szWebServerURL, u64 ullSteamID, const char* szPersonaName, const char* szVersion )
{
char	acBuff[256];
char	acLoginURL[256];
int		nContentLength;
int		nSalt1 = ( rand() << 16 ) | ( rand() );

	msUserLoginPhase = 0;
	strcpy( mszLoginServerBaseURL, szWebServerURL );

	memset( acBuff, 0, 256 );
	sprintf( acBuff, "%08xsteamlogin=%lld&persona=%s&vers=%s", (uint)nSalt1, ullSteamID, szPersonaName, szVersion );
	nContentLength = strlen( acBuff ) + 1;

	// Rounds up to 8 byte blocks..
	nContentLength = UserLoginClientEncrypt( (BYTE*)acBuff, nContentLength  );

	msUserLoginClientLoginState = LOGIN_IN_PROGRESS;

#ifdef LOGIN_WITH_DEV_FLAG_SET
	sprintf( acLoginURL, "%s/SteamLogin?dflag=f1gp8tr", mszLoginServerBaseURL );
#else
	sprintf( acLoginURL, "%s/SteamLogin", mszLoginServerBaseURL );
#endif
	HTTPPost( acLoginURL, (BYTE*)acBuff, nContentLength, UserLoginClientSteamLoginResponseHandler, 0 );
}


void		UserLoginClientLogin( const char* szWebServerURL, const char* szLoginEmail, const char* szPassword, const char* szVersion )
{
char	acBuff[256];
char	acLoginURL[256];
int		nContentLength;
int		nSalt1 = ( rand() << 16 ) | ( rand() );

	msUserLoginPhase = 0;
	strcpy( mszLoginServerBaseURL, szWebServerURL );

	memset( acBuff, 0, 256 );
	sprintf( acBuff, "%08xlogin=%s&pass=%s&vers=%s", (uint)nSalt1, szLoginEmail, szPassword, szVersion );
	nContentLength = strlen( acBuff ) + 1;

	// Rounds up to 8 byte blocks..
	nContentLength = UserLoginClientEncrypt( (BYTE*)acBuff, nContentLength  );

	msUserLoginClientLoginState = LOGIN_IN_PROGRESS;

#ifdef LOGIN_WITH_DEV_FLAG_SET
	sprintf( acLoginURL, "%s/login?dflag=f1gp8tr", mszLoginServerBaseURL );
#else
	sprintf( acLoginURL, "%s/login", mszLoginServerBaseURL );
#endif
	HTTPPost( acLoginURL, (BYTE*)acBuff, nContentLength, UserLoginClientLoginResponseHandler, 0 );

}


void		UserLoginClientUpdate( float fDelta )
{
	// todo

}

int							UserLoginClientGetLoginPhase( void )
{
	return( msUserLoginPhase );
}

eUserLoginClientState		UserLoginClientGetLoginState( void )
{
	return( msUserLoginClientLoginState );
}


const char*		UserLoginClientGetSessionID( void )
{

	return( mszUserLoginClientSessionID );
}

//-------------------------------------------------------------------------------------------
UserLoginClientNewUserDialogMessageCallback		mfnNewUserDialogCallback = NULL;

int		mhNewUserNameTextbox = NOTFOUND;
int		mhNewUserPasswordConfirmTextbox = NOTFOUND;
int		mhNewUserPasswordTextbox = NOTFOUND;
int		mhNewUserLoginEmailTextbox = NOTFOUND;


void		UserLoginClientNewAccountResponseHandler( int nResponseCode, unsigned char* szResponseBody, int nResponseLen, void* vpParam )
{
	switch( nResponseCode )
	{
	case 200:
		mnNewUserLoginStage = NEWUSER_VALID_RESPONSE;
		break;
	case 404:		// notfound = email already in use
		mnNewUserLoginStage = NEWUSER_EMAIL_USED;
		break;
	case 403:		// forbidden = username already in usse
		mnNewUserLoginStage = NEWUSER_USERNAME_TAKEN;
		break;
	default:
		mnNewUserLoginStage = NEWUSER_CREATE_ERROR;
		break;
	}

}

void	UserLoginClientCreateNewUser( const char* szLoginEmail, const char* szPassword, const char* szUsername )
{
char	acBuff[512];
char	acLoginURL[512];
int		nContentLength;
int		nSalt1 = (rand() << 16) | (rand() );

	memset( acBuff, 0, 512 );
	// todo
	sprintf( acBuff, "%08xlogin=%s&pass=%s&name=%s", nSalt1, szLoginEmail, szPassword, szUsername );
	nContentLength = strlen( acBuff ) + 1;

	// Rounds up to 8 byte blocks..
	nContentLength = UserLoginClientEncrypt( (BYTE*)acBuff, nContentLength  );

	sprintf( acLoginURL, "%s/newuser", mszLoginServerBaseURL );
	HTTPPost( acLoginURL, (BYTE*)acBuff, nContentLength, UserLoginClientNewAccountResponseHandler, 0 );

	msUserLoginClientLoginState = LOGIN_CREATE_NEW_USER_IN_PROGRESS;
	mnNewUserLoginStage = NEWUSER_SUBMITTING;
	
}

#ifndef NODISPLAY

void	UserLoginClientNewUserDialogButtonHandler( int nButtonID, ulong ulParam )
{
	switch( ulParam )
	{
	case 0:		// create account
		{
		const char*		pcUsername;
		const char*		pcLoginEmail;
		const char*		pcPassword;
		const char*		pcConfirmPassword;

			pcUsername = UITextBoxGetText( mhNewUserNameTextbox );
			pcLoginEmail = UITextBoxGetText( mhNewUserLoginEmailTextbox );
			pcPassword = UITextBoxGetText( mhNewUserPasswordTextbox );
			pcConfirmPassword = UITextBoxGetText( mhNewUserPasswordConfirmTextbox );

			if ( strcmp( pcPassword, pcConfirmPassword ) != 0 )
			{
				// todo - put up a popup indicating password mismatch
			}
			else if ( strlen( pcUsername ) < 4 ) 
			{
				// todo - put up a popup indicating username too short
			}
			else if ( strlen( pcLoginEmail ) < 6 )
			{
				// todo - put up a popup indicating email invalid
			}
			// todo - further checks.. make sure email addy is valid etc..
			else
			{
				UserLoginClientCreateNewUser( pcLoginEmail, pcPassword, pcUsername );
			}
		}
		break;
	case 1:
		mnNewUserLoginStage = NEWUSER_ENTER_DETAILS;
		break;
	}
}


void		UserLoginClientNewUserDialogInit( const char* szUserLoginServerURL, UserLoginClientNewUserDialogMessageCallback fnCallback )
{
	strcpy( mszLoginServerBaseURL, szUserLoginServerURL );

	mfnNewUserDialogCallback = fnCallback;

	UIRegisterButtonPressHandler( UIRESERVEDBUTTONID_USERLOGINCLIENT_NEWUSER, UserLoginClientNewUserDialogButtonHandler );

	mhNewUserLoginEmailTextbox = UITextBoxCreate( 0, "", 200 );
	mhNewUserNameTextbox = UITextBoxCreate( 0, "", 24 );
	mhNewUserPasswordTextbox = UITextBoxCreate( 1, "", 24 );
	mhNewUserPasswordConfirmTextbox = UITextBoxCreate( 1, "", 24 );
}


void		UserLoginClientNewUserDialogRenderConnecting( int X, int Y, int W, int H, const char* szText, int nMode )
{
int		nTextBoxHeight = (H-50) / 6;
int		nButtonH = (H-50) / 6;
int		nLineY = Y + (H-50) / 6;
int		nTextBoxWidth = W - 200;
int		nTextBoxX = X + 190;
int		nTextOffsetY = 25;

	InterfaceOutlineBox( 2, X, Y, W, H, 0xD0D0D0D0 );
	InterfaceRect( 1, X, Y, W, H, 0xA0000000 );
	InterfaceTextCenter( 1, X, X + W, nLineY, "Create New Account", 0xD0d0b080, 1 );
	nLineY += 40;

	nLineY += (nTextBoxHeight*2);
	InterfaceTextCenter( 1, X, X + W, nLineY, szText, 0xE0F0F0F0, 1 );

	nLineY += (nTextBoxHeight*2);
	UIButtonDraw( UIRESERVEDBUTTONID_USERLOGINCLIENT_NEWUSER, X + 50, nLineY, W - 100, nButtonH, "TRY AGAIN", 0, 1 );
}

void		UserLoginClientNewUserDialogRenderInputDetails( int X, int Y, int W, int H )
{
int		nTextBoxHeight = (H-50) / 6;
int		nButtonH = (H-50) / 6;
int		nLineY = Y + 10;
int		nTextBoxWidth = (int)(W * 0.5f);
int		nTextBoxX = X + (int)(W * 0.3f);
int		nTextOffsetY = (nButtonH / 2) - 11;

	InterfaceOutlineBox( 2, X, Y, W, H, 0xD0D0D0D0 );
	InterfaceRect( 1, X, Y, W, H, 0xA0000000 );
	InterfaceTextCenter( 1, X, X + W, nLineY, "Create New Account", 0xD0d0b080, 1 );
	nLineY += 30;

	InterfaceTextRight( 1, nTextBoxX - 10, nLineY + nTextOffsetY, "Login Email:", 0xD0D0D0D0, 1 );
	UITextBoxRender( mhNewUserLoginEmailTextbox, nTextBoxX, nLineY, nTextBoxWidth, nTextBoxHeight );
	nLineY += nTextBoxHeight + 10;

	InterfaceTextRight( 1, nTextBoxX - 10, nLineY + nTextOffsetY, "Username:", 0xD0D0D0D0, 1 );
	UITextBoxRender( mhNewUserNameTextbox, nTextBoxX, nLineY, nTextBoxWidth, nTextBoxHeight );
	nLineY += nTextBoxHeight + 10;

	InterfaceTextRight( 1, nTextBoxX - 10, nLineY + nTextOffsetY, "Password:", 0xD0D0D0D0, 1 );
	UITextBoxRender( mhNewUserPasswordTextbox, nTextBoxX, nLineY, nTextBoxWidth, nTextBoxHeight );
	nLineY += nTextBoxHeight + 10;

	InterfaceTextRight( 1, nTextBoxX - 10, nLineY + nTextOffsetY, "Confirm Password:", 0xD0D0D0D0, 1 );
	UITextBoxRender( mhNewUserPasswordConfirmTextbox, nTextBoxX, nLineY, nTextBoxWidth, nTextBoxHeight );
	nLineY += nTextBoxHeight + 10;

	UIButtonDraw( UIRESERVEDBUTTONID_USERLOGINCLIENT_NEWUSER, X + 50, nLineY, W - 100, nButtonH, "CREATE ACCOUNT", 0, 0 );
}

const char*		UserLoginClientNewUserGetEmail( void )
{
	return( UITextBoxGetText( mhNewUserLoginEmailTextbox ) );
}

const char*		UserLoginClientNewUserGetPassword( void )
{
	return( UITextBoxGetText( mhNewUserPasswordTextbox ) );
}

BOOL				UserLoginClientHasTicket( ulong ulProductID )
{
	// TODO
	// TEMP - FOR TESTING!! - If the user got any form of TICKET
	// response we'll say they are a member for now.
	if ( mpcTicketResponse )
	{
		return( TRUE );
	}
	return( FALSE );
}


BOOL				UserLoginClientNewUserCreated( void )
{
	if ( mnNewUserLoginStage == NEWUSER_VALID_RESPONSE )
	{
		return( TRUE );
	}
	return( FALSE );
}


void		UserLoginClientNewUserDialogRender( int X, int Y, int W, int H )
{
	switch( mnNewUserLoginStage )
	{
	case NEWUSER_ENTER_DETAILS:
		UserLoginClientNewUserDialogRenderInputDetails( X, Y, W, H );
		break;
	case NEWUSER_SUBMITTING:
		UserLoginClientNewUserDialogRenderConnecting( X, Y, W, H, "Connecting...", 0 );
		break;
	case NEWUSER_USERNAME_TAKEN:
		UserLoginClientNewUserDialogRenderConnecting( X, Y, W, H, "Username already in use", 1 );
		break;
	case NEWUSER_EMAIL_USED:
		UserLoginClientNewUserDialogRenderConnecting( X, Y, W, H, "Email address already in use", 1 );
		break;
	case NEWUSER_CREATE_ERROR:
		UserLoginClientNewUserDialogRenderConnecting( X, Y, W, H, "Unknown error creating account", 1 );
		break;
	case NEWUSER_VALID_RESPONSE:
		UserLoginClientNewUserDialogRenderConnecting( X, Y, W, H, "Logging in...", 1 );
		break;
	}
}

#endif // #ifndef NODISPLAY
