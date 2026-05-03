#ifndef UIX_MENU_H
#define UIX_MENU_H

#include <string>
#include "UIX.h"

class UIXMenuItem : public UIXObject
{
public:
	UIXMenuItem( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void		Initialise( const char* szText, fnSelectedCallback selectedCallback = NULL, uint32 ulSelectParam = 0 );

	const char* GetText() const { return mText.c_str(); }
	UIXMenuItem*		AddMenuItem( const char* szMenuItemName, fnSelectedCallback selectedCallback = NULL, uint32 ulSelectParam = 0);
	void		SetChecked(BOOL bFlag) { mbIsChecked = bFlag; }
	void		SetHeader(BOOL bFlag) { mbIsHeader = bFlag; }

	void		DoRender( InterfaceInstance* pInterface, UIXRECT rect );
	virtual bool		OnSelected( int nButtonID, uint32 ulParam );
	bool			IsExpanded() const { return mbIsExpanded; }

	void		SetSubMenuWidth(int width) { mSubMenuWidth = width; }

	UIXRECT		GetLastRenderRect() const { return mLastRenderRect; }
	void	SetLastRenderRect(UIXRECT rect) { mLastRenderRect = rect; }

protected:
	virtual int		GetSelectionPriorityLayer() { return(GetBasePriority() +  10 ); }
	virtual void		OnEscape();
	virtual void		OnCloseAllMenus();

private:

	void			SetExpanded( bool bFlag ) { mbIsExpanded = bFlag; };

	std::string		mText;
	bool			mbIsExpanded = false;	
	BOOL			mbIsChecked = FALSE;	
	BOOL			mbIsHeader = FALSE;	
	int				mSubMenuWidth = 100;
	UIXRECT			mLastRenderRect;

};

class UIXMenu : public UIXObject
{
public:
	UIXMenu( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void		Initialise( );
	
	UIXMenuItem*		AddMenuItem( const char* szMenuItemName, int nSubMenuWidth = 100 );

	virtual UIXRECT		OnRender( InterfaceInstance* pInterface, UIXRECT rect );
	virtual void		OnPostRender( InterfaceInstance* pInterface, UIXRECT rect );
private:
	virtual int		GetSelectionPriorityLayer() { return(GetBasePriority() +  10 ); }
	virtual bool		ShouldDisplayChildren() { return false; }

};





#endif