#ifndef GAMECOMMON_CONSOLE_H
#define GAMECOMMON_CONSOLE_H

typedef	void(*ConsoleCommandHandler)( char* szParams );


extern void		ConsoleToggleActive( void );
extern void		ConsoleToggleLarge( void );

extern BOOL		ConsoleIsAvailable( void );

extern BOOL		ConsoleIsActive( void );

extern void		ConsoleUpdate( float fDelta );

extern void		ConsoleRender( void );

extern void		ConsoleOnPressEnter( char* szKeyboardInputString );

extern void		ConsolePrint( int mode, const char* szText, ... );

extern void		ConsoleRegisterPrefixCommandHandler( char cPrefixChar, ConsoleCommandHandler fnCallback );

extern void		ConsoleShutdown( void );

//-----------------------------------------------------
// ConsoleCommand Registration
// 
// ConsoleCommand( [Command_name], [Command_func] )
//
// e.g. :
//		void	ConsoleCommandTest( char* szParams )
//		{
//			SysDebugPrint( szParams );
//		}
//		
//		ConsoleCommand( "CmdTest", ConsoleCommandTest );
//
//--------------------------------------------------------------------

class RegisteredConsoleCommandList
{
public:
	static void		Shutdown( void );
	static BOOL		Register( const char* szCommandName, ConsoleCommandHandler fnFunction );

	char*					mszCommandName;
	ConsoleCommandHandler	mfnCommandFunction;

	RegisteredConsoleCommandList*		mpNext;
	
};


#define ConsoleCommand(_textname,_function) \
namespace { \
	static BOOL _function ## _creator_registered = RegisteredConsoleCommandList::Register( _textname, _function); }

#endif
