#include <stdio.h>
#include <stdarg.h>
#include "StandardDef.h"
#include "Interface.h"

#include "../Platform/Platform.h"
#include "Console.h"

BOOL	mbConsoleAvailable = TRUE;
BOOL	mbConsoleActive = FALSE;
BOOL	mbConsoleLarge = FALSE;

#define		MAX_LINES_IN_SUB_CONSOLE_BUFFER		96
#define		NUM_CHARS_IN_CONSOLE_BUFFER_LINE	200

typedef	BOOL(*ConsolePressEnterCallback)( char* );

class CCommonSubConsole
{
public:
	CCommonSubConsole()	{ Init("", NULL, 0); } 
	~CCommonSubConsole()	{ Destroy(); }

	void	Display ( int nX, int nWidthOfBox, int nBottomOfBox, int nNumLines, int nDisplayEdit, float fAlpha );
	void	Print	( const char* szString, int );

	void	Destroy	( void );
	void	Init	( const char* szName,ConsolePressEnterCallback, int  );
	char*	GetName ( void ) { return( acConsoleName ); }
	char*	GetIDName ( void ) { return( acConsoleIDName ); }

	BOOL	HasUnseenText( void ) { return( mboHasUnseenText ); }
	void	ResetUnseenTextFlag( void ) { mboHasUnseenText = FALSE; }

	char*	GetTextLine( int nLine, int* pnCol );

	void	SetToClosing( BOOL bFlag = TRUE );
	BOOL	IsSetToClosing( void ) { return( mboIsSetToClose ); }
private:
	struct CONSOLE_BUFFER
	{
		char	acText[NUM_CHARS_IN_CONSOLE_BUFFER_LINE];
		uint32	uCol;
	};

	CONSOLE_BUFFER		m_aConsoleBuffer[ MAX_LINES_IN_SUB_CONSOLE_BUFFER ];
	int			m_NextSubConsoleLine;
	char		acConsoleName[128];
	char		acConsoleIDName[128];
	BOOL		mboHasUnseenText;
	BOOL		mboIsSetToClose;
	ConsolePressEnterCallback	mfnEnterCallback;
};

CCommonSubConsole		m_sMainConsole;


void	CCommonSubConsole::Init( const char* szName,ConsolePressEnterCallback fnEnterCallback, int nID  )
{
	mboIsSetToClose = FALSE;
	m_NextSubConsoleLine = 0; 
	mfnEnterCallback = fnEnterCallback;
	mboHasUnseenText = FALSE;
	memset( m_aConsoleBuffer, 0, MAX_LINES_IN_SUB_CONSOLE_BUFFER * sizeof(CONSOLE_BUFFER) );
	strcpy( acConsoleName, szName );
}

void	CCommonSubConsole::Destroy( void )
{

}

void	CCommonSubConsole::SetToClosing( BOOL bFlag )
{
	mboIsSetToClose = bFlag;
}



char*	CCommonSubConsole::GetTextLine( int nLine, int* pnCol )
{
int		nActualLineNum;

	nLine %= MAX_LINES_IN_SUB_CONSOLE_BUFFER;
	nActualLineNum = (m_NextSubConsoleLine + (MAX_LINES_IN_SUB_CONSOLE_BUFFER-1)) - nLine;
	if ( nActualLineNum < 0 ) nActualLineNum = 0;
	nActualLineNum %= MAX_LINES_IN_SUB_CONSOLE_BUFFER;

	*pnCol = m_aConsoleBuffer[nActualLineNum].uCol;
	return( m_aConsoleBuffer[nActualLineNum].acText );
}


void	CCommonSubConsole::Display ( int nX, int nWidthOfBox, int nBottomOfBox, int nNumLines, int nDisplayEditMode, float fAlpha )
{
int		nCurrentY;
int		nLoop;
char*	pcNextConsoleLine;
int		nLineNum;
int		nNumLinesToDisplay;
int		nWidth;
const char*	pcEditString = PlatformKeyboardGetInputString( FALSE );
uint32	ulCol;

	mboHasUnseenText = FALSE;
	if ( ( nDisplayEditMode == 2 ) ||
		   ( ( nDisplayEditMode == 1 ) &&
			 ( *pcEditString != 0 ) ) )
	{
		nCurrentY = nBottomOfBox - 12;
		nNumLines--;
	}
	else
	{
		nCurrentY = nBottomOfBox;
	}

	nNumLinesToDisplay = nNumLines;

	nLineNum = m_NextSubConsoleLine;
	for( nLoop = 0; nLoop < nNumLinesToDisplay; nLoop++ )
	{
		nLineNum += (MAX_LINES_IN_SUB_CONSOLE_BUFFER - 1);
		nLineNum %= MAX_LINES_IN_SUB_CONSOLE_BUFFER;

		pcNextConsoleLine = m_aConsoleBuffer[ nLineNum ].acText;
		if ( *(pcNextConsoleLine) != 0 )
		{
			nWidth = GetStringWidth(pcNextConsoleLine, 0 );
			ulCol = GetColWithModifiedAlpha( m_aConsoleBuffer[ nLineNum ].uCol, fAlpha );
			if ( nWidth > nWidthOfBox )
			{
			char*	pcEndOfFirstLine;
				nCurrentY -= 12;
				pcEndOfFirstLine = InterfaceTextLimitWidth( 1, nX, nCurrentY, pcNextConsoleLine, ulCol, 0, nWidthOfBox );
				nCurrentY += 12;
				pcEndOfFirstLine = InterfaceTextLimitWidth( 1, nX, nCurrentY, pcEndOfFirstLine, ulCol, 0, nWidthOfBox );
				nCurrentY -= 24;
				nNumLinesToDisplay--;
			}
			else
			{
				InterfaceText( 1, nX, nCurrentY, pcNextConsoleLine, ulCol, 0 );
				nCurrentY -= 12;
			}
		}
	}

	if ( nDisplayEditMode == 2 )
	{
	char	acBuff[512];	
		sprintf( acBuff, "> %s", pcEditString );
		InterfaceText( 2, nX, nBottomOfBox + 2, acBuff, COL_SYS_SPECIAL, 0 );
	}
	else if ( ( nDisplayEditMode == 1 ) &&
			  ( *pcEditString != 0 ) ) 
	{
		// Add the edit string 
		InterfaceText( 2, nX, nBottomOfBox + 2, pcEditString, COL_SYS_SPECIAL, 0 );
	}
}

void	CCommonSubConsole::Print( const char* szString, int nCol )
{
	strcpy( m_aConsoleBuffer[ m_NextSubConsoleLine ].acText, szString );
	m_aConsoleBuffer[ m_NextSubConsoleLine ].uCol = nCol;

	m_NextSubConsoleLine++;
	m_NextSubConsoleLine %= MAX_LINES_IN_SUB_CONSOLE_BUFFER;
	
	m_aConsoleBuffer[ m_NextSubConsoleLine ].acText[0] = 0;
	mboHasUnseenText = TRUE;
}


//----------------------------------------------

RegisteredConsoleCommandList*	mspConsoleCommandList = NULL;

void	RegisteredConsoleCommandList::Shutdown( void )
{
RegisteredConsoleCommandList*		pRegisteredCommands = mspConsoleCommandList;
RegisteredConsoleCommandList*		pNext;

	while( pRegisteredCommands )
	{
		pNext = pRegisteredCommands->mpNext;
		SAFE_FREE( pRegisteredCommands->mszCommandName );
		delete pRegisteredCommands;

		pRegisteredCommands = pNext;
	}

}

BOOL	RegisteredConsoleCommandList::Register( const char* szCommandName, ConsoleCommandHandler fnFunction )
{
RegisteredConsoleCommandList*		pNewCommand = new RegisteredConsoleCommandList;

	pNewCommand->mszCommandName = (char*)( SystemMalloc( strlen( szCommandName ) + 1 ) );
	strcpy( pNewCommand->mszCommandName, szCommandName );
	pNewCommand->mfnCommandFunction = fnFunction;

	pNewCommand->mpNext = mspConsoleCommandList;
	mspConsoleCommandList = pNewCommand;

	return( TRUE );
}

void		ConsoleShutdown( void )
{
	RegisteredConsoleCommandList::Shutdown();
}

ConsoleCommandHandler		mpfnAsteriskCommandHandler = NULL;
ConsoleCommandHandler		mpfnExclamationCommandHandler = NULL;
ConsoleCommandHandler		mpfnQuestionMarkCommandHandler = NULL;

void		ConsoleRegisterPrefixCommandHandler( char cPrefixChar, ConsoleCommandHandler fnCallback )
{
	if ( cPrefixChar == '*' )
	{
		mpfnAsteriskCommandHandler = fnCallback;
	}
	else if ( cPrefixChar == '!' )
	{
		mpfnExclamationCommandHandler = fnCallback;
	}
	else if ( cPrefixChar == '?' )
	{
		mpfnQuestionMarkCommandHandler = fnCallback;
	}
}

void		ConsoleOnPressEnter( char* szKeyboardInputString )
{
RegisteredConsoleCommandList*		pCommands = mspConsoleCommandList;
char*		pcCommand = szKeyboardInputString;
char*		pcParams;
	

	if ( ( szKeyboardInputString[1] >= 'a' ) &&
		 ( szKeyboardInputString[1] <= 'z' ) )
	{
		if ( ( szKeyboardInputString[0] == '*' ) &&
			 ( mpfnAsteriskCommandHandler ) )
		{
			mpfnAsteriskCommandHandler( pcCommand + 1 );
			return;
		}
		else if ( ( szKeyboardInputString[0] == '!' ) &&
				 ( mpfnExclamationCommandHandler ) )
		{
			mpfnExclamationCommandHandler( pcCommand + 1 );
			return;
		}
		else if ( ( szKeyboardInputString[0] == '?' ) &&
				 ( mpfnQuestionMarkCommandHandler ) )
		{
			mpfnQuestionMarkCommandHandler( pcCommand + 1 );
			return;
		}
	}

	pcParams = SplitStringSep( pcCommand, '.' );
	if ( *pcParams == 0 )
	{
		pcParams = SplitStringSep( pcCommand, ' ' );
	}
	
	while( pCommands )
	{
		if ( stricmp( pCommands->mszCommandName, szKeyboardInputString ) == 0 )
		{
			pCommands->mfnCommandFunction( pcParams );
			return;
		}
		pCommands = pCommands->mpNext;
	}
}

void		ConsoleToggleActive( void )
{
	mbConsoleActive = !mbConsoleActive;
}

void		ConsoleToggleLarge( void )
{
	mbConsoleActive = TRUE;
	mbConsoleLarge = !mbConsoleLarge;
}


BOOL		ConsoleIsAvailable( void )
{
	return( mbConsoleAvailable );
}

BOOL		ConsoleIsActive( void )
{
	return( mbConsoleActive );
}

void		ConsoleUpdate( float fDelta )
{


}

char	mszTempConsolePrint[256] = "";

void		ConsoleRender( void )
{
	if ( mbConsoleActive )
	{
	int			nX, nY, nW, nH;
	const char*		pcInputString;
	int			nNumLines;

		nW = InterfaceGetWidth();
		if ( mbConsoleLarge )
		{
			nH = 600;	
		}
		else
		{
			nH = 120;
		}
		nNumLines = (nH - 5) / 16;
		nX = 0;
		nY = InterfaceGetHeight() - nH;

		InterfaceRect( 0, nX, nY, nW, nH, 0xa0000000 );

		m_sMainConsole.Display( nX + 5, nW - 10, nY + nH - 40, nNumLines, 0, 1.0f );

		InterfaceText( 1, nX + 5, nY + nH - 20, "> ", 0xD0f0e080, 0 );
		pcInputString = PlatformKeyboardGetInputString( TRUE );
		InterfaceText( 1, nX + 25, nY + nH - 20, pcInputString, 0xf0f0e080, 0 );
	}
}



void		ConsolePrint( int mode, const char* text, ... )
{
char		acString[1024];
va_list		marker;
uint32*		pArgs;
//int			nLen;

	pArgs = (uint32*)( &text ) + 1;

	va_start( marker, text );     
	vsprintf( acString, text, marker );

	m_sMainConsole.Print( acString, 0xD0D0D0D0 );
}
