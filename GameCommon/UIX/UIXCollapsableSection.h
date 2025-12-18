#ifndef UIX_COLLAPSABLE_SECTION_H
#define UIX_COLLAPSABLE_SECTION_H

#include <string>
#include "UIX.h"

class UIXCollapsableSection : public UIXObject
{
friend class UIX;
protected:
	UIXCollapsableSection( uint32 uID, UIXRECT rect ) : UIXObject( uID, rect ) {}

	void	Initialise( int mode, const char* szTitle, BOOL bStartCollapsed );
	void	ToggleCollapsed();

	virtual UIXRECT		OnRender( InterfaceInstance* pInstance, UIXRECT pDisplayRect );
	
private:
	virtual bool		ShouldDisplayChildren() { return !mbIsCollapsed; }

	std::string			mTitle;
	BOOL				mbIsCollapsed = FALSE;
	int					mMode = 0;

};






#endif