#ifndef UI_BUTTON_H
#define	UI_BUTTON_H

#include "UI.h"


// ------- externed in UI.h =
//		extern void		UIButtonDraw( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, int nMode, uint32 ulParam );
//		extern BOOL		UIButtonRegion( int nButtonID, int nX, int nY, int nWidth, int nHeight, uint32 ulParam );
//

class ButtonStyle;
class UIInstance;


class UIButtonImpl
{
public:
	UIButtonImpl( UIInstance* pUIInstance );
	void		Shutdown();

	void		Draw( UIInstance* pUIInstance, int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam, float fAlpha );
	void		NewFrame( void );

private:
	ButtonStyle*		mpButtonStyle;

};

extern void		UIButtonsInitialise( void );
extern void		UIButtonsNewFrame( void );
extern void		UIButtonsShutdown( void );

extern void		UIButtonDrawAlphaImpl( UIInstance* pUIInstance, int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam, float fAlpha );


#endif