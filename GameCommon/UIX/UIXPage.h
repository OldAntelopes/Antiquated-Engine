#ifndef UIX_PAGE_H
#define UIX_PAGE_H

#include <string>
#include "UIX.h"

class UIXPage : public UIXObject
{
public:
	UIXPage( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void		Initialise( const char* szTitle, BOOL bUseClipping );
	
	virtual UIXRECT		OnRender( InterfaceInstance* pInterface, UIXRECT rect );
	virtual void		OnUpdate( float delta );
	virtual void		OnPostChildrenRender( InterfaceInstance* pInterface );
	virtual	void		OnMouseWheel( float fAmount ) { if ( mfnMousewheelCallback ) mfnMousewheelCallback(fAmount); }
	UIXRECT		GetPageRenderRect() { return( mPageRenderRect ); }
	BOOL		IsWithinPage(int x, int y);

	void		RegisterMousewheelHandler(fnDefaultMousewheelCallback fnMousewheelCallbackFunc ) { mfnMousewheelCallback = fnMousewheelCallbackFunc; }

	const std::string& GetTitle() const { return mTitle; }	
private:
	UIXRECT		mPageRenderRect;
	BOOL		mbUseClipping = FALSE;
	std::string		mTitle;
	fnDefaultMousewheelCallback		mfnMousewheelCallback = NULL;
};





#endif