#ifndef GAMECOMMON_UI_UIDROPDOWN_H
#define GAMECOMMON_UI_UIDROPDOWN_H

//-----------------------------------------------------
// Externed in UI.h
/*
extern int		UIDropdownCreate( void );
extern int		UIDropdownAddElement( int nHandle, const char* szElementName, ulong ulElementParam );
extern void		UIDropdownRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH );
extern int		UIDropdownGetSelection( char* szElementNameOut, ulong* pulElementParamOut );
extern void		UIDropdownDestroy( int nHandle );
*/

extern int			UIDropdownOnPress( int X, int Y );
extern int			UIDropdownOnRelease( int X, int Y );

extern void			UIDropdownNewFrame( void );

//extern void		UIDropdownUpdate( float fDelta );



#endif // #define GAMECOMMON_UI_UIDROPDOWN_H
