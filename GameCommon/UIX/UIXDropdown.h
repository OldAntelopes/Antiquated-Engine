#ifndef UIX_DROPDOWN_H
#define UIX_DROPDOWN_H

#include <string>
#include <vector>
#include "UIX.h"

#include "UIXScrollableSection.h"

class UIXDropdown;

typedef	void(*fnDropdownSelectionChangedCallback)( UIXObject* pxSourceObject, uint32 ulElementParam );
typedef	int(*fnDropdownUpdateHandler)( UIXObject* pxSourceObject, uint32 ulSelectedElementIndex );

class UIXDropdownEntry
{
friend class UIXDropdown;
public:

	const std::string&		GetText() const { return mTitle; }

protected:
	UIXDropdownEntry( UIXDropdown* pParent, const char* szTitle, uint32 ulUserParam, BOOL bGreyed );

	BOOL		IsGreyed() const { return mbGreyed; }
private:
	std::string						mTitle;
	uint32							mulUserParam = 0;
	UIXDropdown*					mpDropdown;
	BOOL							mbGreyed = FALSE;
};


class UIXDropdown : public UIXObject
{
friend class UIX;
friend class UIXDropdownEntry;
public:
	virtual ~UIXDropdown();
	UIXDropdownEntry*		AddElement(	const char* szElementName, uint32 ulElementParam, BOOL bGreyed = FALSE );

	const UIXDropdownEntry*		GetElementByListIndex( int index ) const;

	void		SetSelectedElementIndex( int index );
	void		SetSelectedElementName( const char* szName );

	void		SetSelectionChangedCallback( fnDropdownSelectionChangedCallback func ) { mSelectionChangedCallback = func; }
	void		RegisterValueUpdateHandler( fnDropdownUpdateHandler func ) { mValueUpdateFunc = func; }
	void		SetDropListWidth( int droplistWidth ) { m_DropListWidth = droplistWidth; }

	virtual void	OnMouseWheel(float fOffset);

protected:
	UIXDropdown( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void	Initialise( int mode );

	virtual UIXRECT		OnRender( InterfaceInstance* pInstance, UIXRECT rect );
	virtual void		OnPostRender( InterfaceInstance* pInterface, UIXRECT rect );

	virtual void		OnUpdate( float fDelta );
	virtual bool		OnSelected( int nButtonID, uint32 ulParam);
	virtual void		OnFocusedKeyUp( int keyCode );
	virtual void		OnCloseAllDropdowns( uint32 ulExceptID );

	void			ToggleExpanded(); 
	
	uint32			GetNextIndex( UIXDropdownEntry* pDropdownEntry ) { mDropdownEntries.push_back( pDropdownEntry ); return( mulNextElementIndex++ ); }
	virtual int		GetSelectionPriorityLayer() { if ( mbIsExpanded ) return( GetBasePriority() + 3 ); return( GetBasePriority() + 1 ); }
	virtual void			OnEscape();

private:

	int					mMode = 0;
	uint32				mulNextElementIndex = 0;
	std::vector<UIXDropdownEntry*>	mDropdownEntries;
	fnDropdownSelectionChangedCallback		mSelectionChangedCallback = NULL;
	UIXDropdownEntry*		mpSelectedEntry = NULL;
	BOOL				mbIsExpanded = FALSE;
	fnDropdownUpdateHandler	mValueUpdateFunc = NULL;
	int		mnSelectedIndex = NOTFOUND;

	int		m_DropListWidth = 0;

	UIXScrollbar* mpScrollbar = NULL;
};






#endif