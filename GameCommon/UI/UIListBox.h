#ifndef UI_LIST_BOX_H
#define UI_LIST_BOX_H


//---------------------------------------- UITextBox
// --- Externed in UI.h
//extern int		UIListBoxCreate( BOOL bContentsDraggable = FALSE );
//extern int		UIListBoxAddElement( int nHandle, const char* szElementName, uint32 ulElementParam );
//extern void		UIListBoxRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH, float fAlpha );
//extern int		UIListBoxGetSelection( int nHandle, char* szElementNameOut, uint32* pulElementParamOut );
//extern int		UIListBoxGetNumElements( int nHandle );


//------------------------------- UI Internal

extern BOOL		UIListBoxOnRelease( int X, int Y );
extern BOOL		UIListBoxOnPress( int X, int Y );

extern void		UIListBoxNewFrame( void );

extern void		UITextboxShutdown( void );

extern void		UITextBoxEndCurrentEdit( void );

#endif