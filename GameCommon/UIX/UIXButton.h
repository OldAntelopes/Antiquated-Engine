#ifndef UIX_BUTTON_H
#define UIX_BUTTON_H

#include <string>
#include "UIX.h"

class UIXButton : public UIXObject
{
friend class UIX;
protected:
	UIXButton( uint32 uID, UIXRECT rect ) : UIXObject( uID, rect ) {}

	void	Initialise( int mode, const char* szTitle, uint32 ulButtonID, uint32 ulButtonParam );
	
	virtual UIXRECT		OnRender( InterfaceInstance* pInstance, UIXRECT pDisplayRect );

private:
	std::string			mTitle;
	uint32				mulButtonID = 0;
	uint32				mulButtonParam = 0;
	int					mMode = 0;

};






#endif