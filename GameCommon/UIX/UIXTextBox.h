#ifndef UIX_TEXT_BOX_H
#define UIX_TEXT_BOX_H

#include <string>
#include "UIX.h"

typedef	void(*fnTextBoxEndEditCallback)( UIXObject* pxSourceObject, const char* szNewText );

class UIXTextBox : public UIXObject
{
friend class UIX;
friend class UIXSlider;
public:
	void	SetText( const char* szText, ... );
	const std::string&		GetText() const { return mText; }

	void	SetEndEditCallback( fnTextBoxEndEditCallback func ) { mfnEndEditCallback = func; }
protected:
	UIXTextBox( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void	Initialise( int mode, const char* szTitle );
	
	virtual UIXRECT		OnRender( InterfaceInstance* pInterface, UIXRECT pDisplayRect );
	static void			RegisterControlHandlers();
	virtual bool		OnSelected( int nButtonID, uint32 ulParam );
	virtual void		EndEdit();
	virtual int		GetSelectionPriorityLayer() { return(GetBasePriority() + 1); }

private:
	static int		TextBoxKeyboardMessageHandlerStatic( int nResponseCode, const char* szInputText, void* pUserObj );
	
	int		OnKeyboardMessage( int nResponseCode, const char* szInputText );
	BOOL	OnSelect();

	std::string			mText;
	std::string			mPreEditText;
	int					mMode;
	fnTextBoxEndEditCallback		mfnEndEditCallback = NULL;
};






#endif