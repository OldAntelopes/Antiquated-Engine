#ifndef UIX_SLIDER_H
#define UIX_SLIDER_H

#include <string>
#include "UIX.h"

class UIXTextBox;

class UIXSlider : public UIXObject
{
friend class UIX;
public:

	void	RegisterValueUpdateHandler( fnValueUpdateCallback func ) { mValueUpdateFunc = func; }
	void	SetRangeLimit( float min, float max ) { mfInitialMinVal = min; mfInitialMaxVal = max; }
	void	AllowRangeExpand( BOOL bFlag ) { mbAllowRangeExpand = bFlag; }
	void	ShowCurrentValueIndicator( BOOL bFlag ) { mbShowCurrentValueIndicator = bFlag; }

	virtual float		OnValueChange( UIXObject* pxSourceObj, float fNewValue, BOOL bByUserEditFlag );		// Triggered by (e.g) child text boxes when a new value is entered there directly

	void	ModifySliderRange( float fValue, float fMinRange, float fMaxRange );		// Triggered by (e.g) midi control changing slider values

protected:
	UIXSlider( UIXObject* pxParent, uint32 uID, UIXRECT rect ) : UIXObject( pxParent, uID, rect ) {}

	void	Initialise( UIX_SLIDER_MODE mode,uint32 ulUserParam, float fMin, float fMax, float fInitialVal, float fMinStep, const char* szText, BOOL bShowValueTextBoxes );
	
	void	OnHeldUpdate( BOOL bIsHeld, BOOL bFirstPress );
	void	OnMinRangeHeldUpdate( BOOL bIsHeld, BOOL bFirstPress );
	void	OnMaxRangeHeldUpdate( BOOL bIsHeld, BOOL bFirstPress );

	virtual UIXRECT		OnRender( InterfaceInstance* pInstance, UIXRECT pDisplayRect );
	virtual void		OnUpdate( float fDelta );

private:
	virtual bool		ShouldDisplayChildren() { return false; }

	float				GetCursorValue();
	UIXRECT				mRenderRect;
	UIX_SLIDER_MODE		mMode = SLIDERMODE_VALUE;
	float				mfMinVal = 0.0f;
	float				mfMaxVal = 1.0f;
	float				mfInitialMinVal = 0.0f;
	float				mfInitialMaxVal = 1.0f;
	float				mfInitialVal = 0.0f;
	float				mfCurrentVal = 0.0f;
	float				mfMinStep = 0.1f;
	uint32				mulUserParam = 0;
	BOOL				mbIsHeld = FALSE;
	BOOL				mbAllowRangeExpand = TRUE;
	BOOL				mbShowValueTextBoxes = TRUE;
	BOOL				mbShowCurrentValueIndicator = TRUE;
	std::string			mText;

	fnValueUpdateCallback	mValueUpdateFunc = NULL;

	UIXTextBox*			mpValueTextBox = NULL;
	UIXTextBox*			mpRangeMinTextBox = NULL;
	UIXTextBox*			mpRangeMaxTextBox = NULL;
};






#endif