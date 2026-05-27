#ifndef UIX_DIAL_H
#define UIX_DIAL_H

#include <string>
#include "UIX.h"


class UIXDial : public UIXObject
{
friend class UIX;
public:

	void	RegisterValueUpdateHandler( fnValueUpdateCallback func ) { mValueUpdateFunc = func; }

protected:
	UIXDial( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void	Initialise( uint32 ulUserParam, float fInitialVal );
	
	void	OnHeldUpdate( BOOL bIsHeld, BOOL bFirstPress );

	virtual UIXRECT		OnRender( InterfaceInstance* pInstance, UIXRECT pDisplayRect );
	virtual void		OnUpdate( float fDelta );

private:
	virtual bool		ShouldDisplayChildren() { return false; }

	UIXRECT				mRenderRect;
	float				mfCurrentVal = 0.0f;
	uint32				mulUserParam = 0;
	BOOL				mbIsHeld = FALSE;

	fnValueUpdateCallback	mValueUpdateFunc = NULL;
};






#endif