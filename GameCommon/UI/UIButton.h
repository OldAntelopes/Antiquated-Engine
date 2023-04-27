#ifndef UI_BUTTON_H
#define	UI_BUTTON_H


// ------- externed in UI.h =
//		extern void		UIButtonDraw( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, int nMode, uint32 ulParam );
//		extern BOOL		UIButtonRegion( int nButtonID, int nX, int nY, int nWidth, int nHeight, uint32 ulParam );
//


extern void		UIButtonsInitialise( void );
extern void		UIButtonsNewFrame( void );
extern void		UIButtonsShutdown( void );



#endif