#ifndef UI_SCROLLABLE_PAGE_H
#define UI_SCROLLABLE_PAGE_H

// --- Externed in UI.h
//extern int		UIScrollablePageCreate( void );
//extern void		UIScrollablePageRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH );
//extern void		UIScrollablePageRenderHorizontal( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullW );
//extern int		UIScrollablePageGetPosition( int nHandle );
//extern void		UIScrollablePageDestroy( int nHandle );


//------------------------------- UI Internal


int			UIScrollablePageOnPress( int X, int Y );
int			UIScrollablePageOnRelease( int X, int Y );
void		UIScrollablePageUpdate( float fDelta );
void		UIScrollablePageOnZoom( float fZoomAmount );

#endif
