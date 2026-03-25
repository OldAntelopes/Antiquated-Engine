#ifndef UIX_SHAPE_H
#define UIX_SHAPE_H

#include <string>
#include "UIX.h"

class UIXShape : public UIXObject
{
friend class UIX;
public:
	void	SetAnimation( int animMode, float fTime, float fCurrPhase = 0.0f ) { mnAnimationMode = animMode; mfAnimationTime = fTime; mfAnimationPhase = fCurrPhase; }
protected:
	UIXShape( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void	Initialise( eUIXSHAPE_MODE mode, BOOL bBlocks,uint32 ulCol1, uint32 ulCol2, uint32 ulButtonID, uint32 ulButtonParam);
	
	virtual void		OnUpdate( float delta );
	virtual UIXRECT		OnRender( InterfaceInstance* pInterface, UIXRECT pDisplayRect );

private:
	uint32				mulCol1 = 0xC0C0C0C0;
	uint32				mulCol2 = 0xC0C0C0C0;
	eUIXSHAPE_MODE		mMode = UIXSHAPE_SHADEDRECT;
	BOOL				mBlocks = FALSE;
	uint32				mulButtonID = 0;
	uint32				mulButtonParam = 0;
	float				mfAnimationTime = 0.0f;
	float				mfAnimationPhase = 0.0f;
	int					mnAnimationMode = 0;
};






#endif