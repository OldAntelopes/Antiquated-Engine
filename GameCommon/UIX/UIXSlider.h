#ifndef UIX_SLIDER_H
#define UIX_SLIDER_H

#include "UIX.h"

class UIXSlider : public UIXObject
{
friend class UIX;
public:

	void	RegisterValueUpdateHandler( fnValueUpdateCallback func ) { mValueUpdateFunc = func; }

protected:
	UIXSlider( uint32 uID, UIXRECT rect ) : UIXObject( uID, rect ) {}

	void	Initialise( UIX_SLIDER_MODE mode,uint32 ulUserParam, float fMin, float fMax, float fInitialVal, float fMinStep );
	
	void	OnHeldUpdate( BOOL bIsHeld, BOOL bFirstPress );

	virtual UIXRECT		OnRender( InterfaceInstance* pInstance, UIXRECT pDisplayRect );
	virtual void		OnUpdate( float fDelta );

private:
	UIXRECT				mRenderRect;
	UIX_SLIDER_MODE		mMode;
	float				mfMinVal;
	float				mfMaxVal;
	float				mfInitialVal;
	float				mfCurrentVal;
	float				mfMinStep;
	uint32				mulUserParam;
	BOOL				mbIsHeld;

	fnValueUpdateCallback	mValueUpdateFunc = NULL;
};






#endif