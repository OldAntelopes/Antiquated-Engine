
#include "s3eOSReadString.h"
#include "s3eKeyboard.h"

#include "StandardDef.h"

#include "ClientConnectionToNewChatServer.h"
#include "ChatLog.h"
#include "KeyboardInput.h"
#include "MainDisplay.h"

#define		KEYBOARDTYPE1


// Display last 5 characters generated
#define NUM_CHARS 256
static s3eWChar g_Chars[NUM_CHARS];
static int g_NumChars = 0;

// Last 5 keys pressed down
static s3eKey g_Keys[NUM_CHARS];
static int g_NumKeys = 0;

// Last 5 keys released
static s3eKey g_KeysReleased[NUM_CHARS];
static int g_NumKeysReleased = 0;

// Last key that was "pressed" (has just been released)
static s3eKey g_LastKeyPressed = s3eKeyFirst;

static int g_CharEnabled = 0;


// Callback event gets called whenever a key state changes
static int32 handler(void* sys, void*)
{
s3eKeyboardEvent* event = (s3eKeyboardEvent*)sys;

    if (event->m_Pressed) // a key state changed from up to down
    {
        if (g_NumKeys < NUM_CHARS)
		{
            g_NumKeys++;
		}
		
		// Move previous entries down through the array and add new one at end
		memmove(g_Keys+1, g_Keys, (NUM_CHARS - 1) * sizeof(s3eKey));
        g_Keys[0] = event->m_Key;
    }
    else // state changed from down to up
    {
        if (g_NumKeysReleased < NUM_CHARS)
            g_NumKeysReleased++;

        memmove(g_KeysReleased+1, g_KeysReleased, (NUM_CHARS - 1) * sizeof(s3eKey));
        g_KeysReleased[0] = event->m_Key;
    }
    return 0;
}

char	mszInputBuffer[256];

const char*		KeyboardGetInputText( void )
{
int		nLoop;
char*	pcOut = mszInputBuffer;
s3eWChar	wChar;

	for ( nLoop = 0; nLoop < g_NumChars; nLoop++ )
	{
		*pcOut = (char)( g_Chars[nLoop] );
		pcOut++;
	}
	*pcOut = 0;
	return( mszInputBuffer );
}


void		KeyboardInputUpdate( float fDelta )
{
    // Check for character input being terminated by the platform/user
    int newCharState = s3eKeyboardGetInt(S3E_KEYBOARD_GET_CHAR);

    s3eWChar ch;

	if (g_CharEnabled)
    {
        // Get all chars that were internally buffered since the last update
        while ((ch = s3eKeyboardGetChar()) != S3E_WEOF)
        {
			switch ( ch )
			{
			case 8: // backspace
				if ( g_NumChars > 0 )
				{
					g_NumChars--;
					g_Chars[g_NumChars] = 0;
				}
				break;
			default:
				if ( ch >= 32 )
				{
					g_Chars[g_NumChars] = ch;
					if (g_NumChars < (NUM_CHARS-1))
						g_NumChars++;
				}
				break;
			}
        }
    }

    // Get the last key that was pressed. Will be return s3eKeyFirst if no key
    // was pressed between the previous calls to s3eKeyboardUpdate().
    g_LastKeyPressed = s3eKeyboardAnyKey();

	if ( g_LastKeyPressed == s3eKeyEnter )
	{
	const char*		pcInputText = KeyboardGetInputText();

		MainDisplayOnEnterText( pcInputText );
		g_NumChars = 0;
		g_Chars[g_NumChars] = 0;
		g_CharEnabled = 0;
		s3eKeyboardSetInt( S3E_KEYBOARD_GET_CHAR, false );
	}
}

BOOL		PlatformKeyboardOSKIsActive( void )
{
	if ( g_CharEnabled == 1 )
	{
		return( TRUE );
	}
	return( FALSE );
}

void		KeyboardInputInit( void )
{
    s3eKeyboardRegister(S3E_KEYBOARD_KEY_EVENT, handler, NULL);

	if ( s3eOSReadStringAvailable() == S3E_TRUE )
	{


	}
}


void		PlatformKeyboardActivate( int mode, const char* szStartText, const char* szPrompt )
{
	g_NumChars = 0;
	if ( szStartText )
	{
	const char*		pcRunner = szStartText;
		while( *pcRunner != 0 )
		{
			g_Chars[g_NumChars] = *pcRunner++;
			g_NumChars++;
		}
	}
	g_CharEnabled = 1;
	s3eKeyboardSetInt( S3E_KEYBOARD_GET_CHAR, false );
	s3eKeyboardSetInt( S3E_KEYBOARD_GET_CHAR, true );
}

