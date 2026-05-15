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
	virtual void		OnPostChildrenRender( InterfaceInstance* pInterface );

	UIXRECT		GetPageRenderRect() { return( mPageRenderRect ); }
	BOOL		IsWithinPage(int x, int y);

	const std::string& GetTitle() const { return mTitle; }	
private:
	UIXRECT		mPageRenderRect;
	BOOL		mbUseClipping = FALSE;
	std::string		mTitle;

};





#endif