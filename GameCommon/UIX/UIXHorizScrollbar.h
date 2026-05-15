#ifndef UIX_HORIZ_SCROLLBAR_H
#define UIX_HORIZ_SCROLLBAR_H

#include "UIX.h"

class UIXHorizScrollbar : public UIXObject
{
friend class UIX;
public:


protected:
	virtual ~UIXHorizScrollbar();
	UIXHorizScrollbar( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void	Initialise( fnControlCallback controlCallback, int nVal, int nViewRange, int nMaxRange );
	
	void	OnHeldUpdate( BOOL bIsHeld, BOOL bFirstPress );
	void	OnMinRangeHeldUpdate( BOOL bIsHeld, BOOL bFirstPress );
	void	OnMaxRangeHeldUpdate( BOOL bIsHeld, BOOL bFirstPress );

	BOOL			HoldHandler( uint32 ulElementIndex, BOOL bIsHeld, BOOL bFirstPress );
	static BOOL		HoldHandlerStatic( int nButtonID, uint32 ulParam, uint32 ulIndex, BOOL bIsHeld, BOOL bFirstPress );
	static void		RegisterControlHandlers();

	virtual UIXRECT		OnRender( InterfaceInstance* pInstance, UIXRECT pDisplayRect );
	virtual void		OnUpdate( float fDelta );

private:
	void	RenderScrollbar( InterfaceInstance* pInterface, UIXRECT rect);
	virtual bool		ShouldDisplayChildren() { return false; }

	int			mhScrollable = NOTFOUND;
	fnControlCallback		mfnControlCallback = NULL;
	int			mnViewRange = 10;
	int			mnMaxRange = 10;
	int			mnScrollPosition = 0;

	UIXRECT		mScrollbarLastRender;
	int			mnHoverOffsetX = 0;
	int			mnPressPosScreenX = 0;
	int			mnPressPosOffsetX = 0;
	int			mnHoldStartScrollPosScreen = 0;
	int			mnHoldStartScrollPos = 0;
	int			mnHoldMoveDistanceX = 0;
	BOOL		mbDidGrabScrollbar = FALSE;
};


#endif