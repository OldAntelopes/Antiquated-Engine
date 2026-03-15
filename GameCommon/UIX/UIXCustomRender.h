#ifndef UIX_CUSTOMRENDER_H
#define UIX_CUSTOMRENDER_H

#include <string>
#include "UIX.h"


class UIXCustomRender : public UIXObject
{
friend class UIX;
protected:
	UIXCustomRender( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void	Initialise( fnCustomRenderCallback renderFunc, uint32 ulUserParam, fnCustomDragHoldHandlerCallback dragFunc = NULL);
	
	virtual UIXRECT		OnRender( InterfaceInstance* pInstance, UIXRECT pDisplayRect );
	virtual BOOL		OnDragHoldUpdate(uint32 ulIndex, BOOL bIsHeld, BOOL bFirstPress);

private:
	virtual bool		IncludeChildrenInOccupyCalc() { return false; }

	fnCustomRenderCallback		mfnRenderCallback = NULL;
	fnCustomDragHoldHandlerCallback		mfnCustomDragHolderHandlerCallback = NULL;
	uint32			mulUserParam = 0;



};






#endif