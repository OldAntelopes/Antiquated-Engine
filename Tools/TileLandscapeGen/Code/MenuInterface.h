#ifndef TILEGEN_MENU_INTERFACE_H
#define TILEGEN_MENU_INTERFACE_H



extern void		MenuInterfaceInit( void );

extern void		MenuInterfaceUpdate( float fDelta );

extern void		MenuInterfaceRender( void );

extern void		MenuInterfaceShutdown( void );

extern void		MenuInterfacePrint( const char* szText, ... );

extern const char*		MenuInterfaceGetLevelFolderPath( void );


#endif