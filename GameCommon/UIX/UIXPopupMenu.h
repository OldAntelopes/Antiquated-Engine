#ifndef UIX_POPUP_MENU_H
#define UIX_POPUP_MENU_H

#include <string>
#include <vector>

#include "UIX.h"

class UIXPopupMenuItem : public UIXObject
{
public:
	UIXPopupMenuItem( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void		Initialise( const char* szText, fnSelectedCallback selectedCallback = NULL, uint32 ulSelectParam = 0 );
	void		SetCheckbox( BOOL bIsCheckbox, BOOL bState ) { mbIsCheckbox = bIsCheckbox; mbIsChecked = bState; }
	void		SetHeader( BOOL bIsHeader ) { mbIsHeader = bIsHeader; }
	void		OnHover();

	int		Display( InterfaceInstance* pInterface, UIXRECT lineRect );

	UIXPopupMenuItem*		AddHeaderMenuItem( const char* szMenuItemName, ... );
	UIXPopupMenuItem*		AddMenuItem( const char* szMenuItemName, fnSelectedCallback selectedCallback = NULL, uint32 ulSelectParam = 0, BOOL bIsCheckbox = FALSE, BOOL bIsChecked = FALSE );
	
	const char* GetText() const { return mText.c_str(); }
		
	void			DoRender( InterfaceInstance* pInterface, UIXRECT rect );
	virtual bool	OnSelected( int nButtonID, uint32 ulParam );
	bool			IsExpanded() const { return mbIsExpanded; }
	BOOL			IsChecked() const { return mbIsChecked; }
	BOOL			IsCheckbox() const { return mbIsCheckbox; }
	BOOL			IsHeader() const { return mbIsHeader; }

	UIXRECT		GetLastRenderRect() const { return mLastRenderRect; }
	void		SetLastRenderRect(UIXRECT rect) { mLastRenderRect = rect; }

	void		SetExpanded( BOOL bFlag );

protected:
	virtual int			GetSelectionPriorityLayer() { return( GetBasePriority() + 10 ); }

private:

	void			SetExpanded( bool bFlag ) { mbIsExpanded = bFlag; };

	std::string		mText;
	bool			mbIsExpanded = false;	
	BOOL			mbIsChecked = FALSE;
	BOOL			mbIsCheckbox = FALSE;
	BOOL			mbIndentForCheckbox = FALSE;
	BOOL			mbIsHeader = FALSE;
	UIXRECT			mLastRenderRect;
	float			mfExpandDelayTime = 0.0f;
};


class UIXPopupMenu : public UIXObject
{
friend class UIXPopupMenuItem;
public:
	UIXPopupMenu( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void		Initialise( );
	
	UIXPopupMenuItem*		AddHeaderMenuItem( const char* szMenuItemName, ... );
	UIXPopupMenuItem*		AddMenuItem( const char* szMenuItemName, fnSelectedCallback selectedCallback = NULL, uint32 ulSelectParam = 0, BOOL bIsCheckbox = FALSE, BOOL bIsChecked = FALSE  );

	virtual UIXRECT		OnRender( InterfaceInstance* pInterface, UIXRECT rect );
	virtual void		OnPostRender( InterfaceInstance* pInterface, UIXRECT rect );

protected:
	void			ClearExpandedChildren();
private:
	virtual int		GetSelectionPriorityLayer() { return(GetBasePriority() +  10 ); }
	virtual bool		ShouldDisplayChildren() { return false; }

};


#endif // CONTROL_PANEL_INTERFACES_GENERIC_POPUP_MENU_H