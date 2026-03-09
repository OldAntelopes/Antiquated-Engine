#ifndef UI_TEXT_BOX_H
#define UI_TEXT_BOX_H

class UITextBox;
class UIInstance;

//---------------------------------------- UITextBox
// --- Externed in UI.h
//extern int			UITextBoxCreate( int nMode, const char* szInitialText, int nMaxTextLen );
//extern void			UITextBoxRender( int nHandle, int nScreenX, int nScreenY, int nScreenW, int nScreenH );
//extern const char*	UITextBoxGetText( int nHandle );
//extern void			UITextBoxDestroy( int nHandle );


//------------------------------- UI Internal
class UITextBoxImpl
{
public:
	UITextBoxImpl( UIInstance* pUIInstance );
	void		Shutdown();

//	void		Draw( UIInstance* pUIInstance, int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam, float fAlpha );
	void		NewFrame( void );
		
	int			Create( int nMode, const char* szInitialText, int nMaxTextLen );
	void		Render( int nHandle, int nScreenX, int nScreenY, int nScreenW, int nScreenH );
	const char*	GetText( int nHandle );
	void		EndEdit( int nHandle );
	void		Destroy( int nHandle );
	
	int			KeyboardMessageHandler( int nResponseCode, const char* szInputText, void* pUserObj );
	void		EndCurrentEdit( void );
	BOOL		OnRelease( int X, int Y );


private:
	int				msnTextBoxNextHandle = 401;
	UITextBox*		mspTextBoxList = NULL;

	UITextBox*		mspTextBoxHover = NULL;
	UITextBox*		mspFocusedTextBox = NULL;
};



extern BOOL		UITextBoxOnRelease( int X, int Y );

extern void		UITextBoxNewFrame( void );

extern void		UITextboxShutdown( void );

extern void		UITextBoxEndCurrentEdit( void );

#endif