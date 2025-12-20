#ifndef UIX_CUSTOMRENDER_H
#define UIX_CUSTOMRENDER_H

#include <string>
#include "UIX.h"

class UIXCustomRender : public UIXObject
{
friend class UIX;
protected:
	UIXCustomRender( uint32 uID, UIXRECT rect ) : UIXObject( uID, rect ) {}

	void	Initialise( fnCustomRenderCallback renderFunc, uint32 ulUserParam1, uint32 ulUserParam2 );
	
	virtual UIXRECT		OnRender( InterfaceInstance* pInstance, UIXRECT pDisplayRect );

private:
	fnCustomRenderCallback		mfnRenderCallback;
	uint32			mulUserParam1;
	uint32			mulUserParam2;

};






#endif