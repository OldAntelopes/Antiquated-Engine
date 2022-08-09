#ifndef UI_TEXT_BOX_H
#define UI_TEXT_BOX_H


//---------------------------------------- UITextBox
// --- Externed in UI.h
//extern int			UITextBoxCreate( int nMode, const char* szInitialText, int nMaxTextLen );
//extern void			UITextBoxRender( int nHandle, int nScreenX, int nScreenY, int nScreenW, int nScreenH );
//extern const char*	UITextBoxGetText( int nHandle );
//extern void			UITextBoxDestroy( int nHandle );


//------------------------------- UI Internal

extern BOOL		UITextBoxOnRelease( int X, int Y );

extern void		UITextBoxNewFrame( void );

extern void		UITextboxShutdown( void );

extern void		UITextBoxEndCurrentEdit( void );

#endif