#ifndef UI_SLIDER_H
#define UI_SLIDER_H



// --- Externed in UI.h
//extern int		UISliderCreate( int nValue, int nMin, int nMax );
//extern void		UISliderDraw( int nHandle, int nX, int nY, int nWidth, int nHeight, int nFlags );
//extern int		UISliderGetValue( int nHandle );
//extern void		UISliderSetValue( int nHandle, int nValue );
//extern void		UISliderDestroy( int nHandle );


//------------------------------- UI Internal

extern BOOL		UISliderOnPress( int X, int Y );
extern BOOL		UISliderOnRelease( int X, int Y, BOOL bFocused );




#endif