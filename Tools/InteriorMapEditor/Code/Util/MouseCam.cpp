
#include <math.h>
#include <windows.h>
#include <StandardDef.h>
#include <Engine.h>

#include <Interface.h>
#include "MouseCam.h"


VECT	mxMousePos;
VECT	mxMouseDownPos;
VECT	mxMouseLastPos;
BOOL	mboLeftMouseDown = FALSE;
BOOL	mboRightMouseDown = FALSE;
BOOL	mboMidMouseDown = FALSE;
float	mfMouseCamDist = 1.0f;
float	mfOriginalCamDist = 1.0f;

VECT		mxBaseMouseCamFocus  = { 0.0f, 0.0f, 0.05f };
VECT		mxMouseCamFocus  = { 0.0f, 0.0f, 0.05f };
VECT		mxMouseCamPos  = { 0.0f, 1.0f, 0.0f };
VECT		mxMouseCamView = { 0.0f, -1.0f, 0.0f };

VECT		mxMouseCamStandardPos  = { 0.0f, 1.0f, 0.0f };
VECT		mxMouseCamStandardView = { 0.0f, -1.0f, 0.0f };

VECT	mxOriginalCamView;
float	mfOriginalViewAngleVert;
float	mfViewAngleVert = 0.0f;
float	mfOriginalViewAngleHoriz;
float	mfViewAngleHoriz = 0.0f;
float	mfWheelTurnRot = 0.0f;
int		mnMouseCamControlMode = CONTROL_ROT;
int		mnMouseCamViewMode = VIEW_MODE_NORMAL;

#define	ROT_BASE		0.002f
#define	MOVE_BASE		0.0002f
#define	ZOOM_BASE		0.01f

float	mfRotSpeed = ROT_BASE;
float	mfMoveSpeed = MOVE_BASE;
float	mfZoomSpeed = ZOOM_BASE;

void	MouseCamSetMoveSpeed( float fSpeed )
{
	mfMoveSpeed = fSpeed * MOVE_BASE;
}

void	MouseCamSetRotSpeed( float fSpeed )
{
	mfRotSpeed = fSpeed * ROT_BASE;
}

void	MouseCamSetZoomSpeed( float fSpeed )
{
	mfZoomSpeed = fSpeed * ZOOM_BASE;
}

int		MouseCamGetControlMode( void )
{
	return( mnMouseCamControlMode );
}
void	MouseCamSetControlMode( int Mode )
{
	mnMouseCamControlMode = Mode;
}

void	MouseCamSetViewMode( int Mode)
{
	switch( mnMouseCamViewMode )
	{
	case VIEW_MODE_NORMAL:
		mxMouseCamStandardPos  = mxMouseCamPos;
		mxMouseCamStandardView = mxMouseCamView;
		break;
	}
	mnMouseCamViewMode = Mode;
	switch( Mode )
	{
	case VIEW_MODE_TOPDOWN:
	
		mfMouseCamDist = 1.0f;
		mxMouseCamPos.x = 0.0f;//mxMouseCamFocus.x;
		mxMouseCamPos.y = 0.0f;//mxMouseCamFocus.y;
		mxMouseCamPos.z = 2.0f;//mxMouseCamFocus.z + mfMouseCamDist;
		mxMouseCamView.x = 0.0f;
		mxMouseCamView.y = 0.0f;
		mxMouseCamView.z = -1.0f;
		break;
	case VIEW_MODE_NORMAL:
		mxMouseCamPos = mxMouseCamStandardPos;
		mxMouseCamView = mxMouseCamStandardView;
		break;
	}
	MouseCamUpdateCameraMatrices();
}


/***************************************************************************
 * Function    : MouseCamUpdateCameraMatrices
 * Params      :
 * Description : 
 ***************************************************************************/
void		MouseCamUpdateCameraMatrices( void )
{
ENGINEMATRIX matRot1;
ENGINEMATRIX matRot2;
float	fNearClipPlane = 0.01f;
float	fFarClipPlane = 200.0f;
float	fFOV = (float)(PI/4);


	switch( mnMouseCamViewMode )
	{
	case VIEW_MODE_TOPDOWN:
		EngineCameraSetPos( 0.0f,0.0f,1.0f+mfMouseCamDist );//mxMouseCamPos.x, mxMouseCamPos.y, mxMouseCamPos.z );
		EngineCameraSetDirection( 0.0f, 0.0f, -1.0f );
		EngineCameraSetUpVect( 0.0f, 1.0f, 0.0f );
		EngineCameraUpdate();
		break;
	default:
		mxMouseCamView.x = 0.0f;
		mxMouseCamView.y = 1.0f;
		mxMouseCamView.z = 0.0f;
		EngineMatrixRotationX( &matRot1, mfViewAngleVert );
		EngineMatrixRotationZ( &matRot2, mfViewAngleHoriz );
		VectTransform( &mxMouseCamView, &mxMouseCamView, &matRot1 );
		VectTransform( &mxMouseCamView, &mxMouseCamView, &matRot2 );

		EngineCameraSetPos( mxMouseCamPos.x, mxMouseCamPos.y, mxMouseCamPos.z );
		EngineCameraSetDirection( mxMouseCamView.x,mxMouseCamView.y,mxMouseCamView.z );

		EngineCameraSetUpVect( 0.0f, 0.0f, 1.0f );
		EngineCameraUpdate();
		break;
	}
}

void MouseCamMidMouseDownMove( float fScreenXDelta, float fScreenYDelta, float fTotalXDelta, float fTotalYDelta )
{
	mxMouseCamFocus.z = mxBaseMouseCamFocus.z + (fScreenYDelta * 0.01f);
//	LevelEdCamSetTransScreenDelta( fScreenXDelta, fScreenYDelta );
}


void MouseCamSet( VECT* pxPos, VECT* pxFocus )
{
VECT		xVect;
VECT		xVectNormalized;

	mxMouseCamFocus = *pxFocus;
	VectSub( &xVect, pxFocus, pxPos );
	
	mxMouseCamView = xVect;
	mfMouseCamDist = VectGetLength( &xVect );
	mxBaseMouseCamFocus = *pxFocus;
	mxMouseCamPos = *pxPos;

	VectNormalize( &xVect );
	xVectNormalized = xVect;
	xVectNormalized.z = 0.0f;
//	VectNormalize( &xVectNormalized );
	mfOriginalViewAngleHoriz = atan2f( xVectNormalized.y, xVectNormalized.x ) + A90;
	xVectNormalized = xVect;
//	xVectNormalized.x += xVectNormalized.y;
	xVectNormalized.y = 0.0f;
//	VectNormalize( &xVectNormalized );
	mfOriginalViewAngleVert = atan2f( xVectNormalized.x, xVectNormalized.z );
/*
	mfOriginalViewAngleHoriz = A180 - GetViewRotationFromVector( (MVECT*)&xVect );
	xVect.y = xVect.z;
	xVect.z = 0.0f;
	mfOriginalViewAngleVert = GetViewRotationFromVector( (MVECT*)&xVect );
*/

	mfViewAngleVert = mfOriginalViewAngleVert;
	mfViewAngleHoriz = mfOriginalViewAngleHoriz;

}

void MouseCamMouseDownStore( void )
{
	mfOriginalCamDist = mfMouseCamDist;
	mfOriginalViewAngleVert = mfViewAngleVert;
	mfOriginalViewAngleHoriz = mfViewAngleHoriz;
	mxBaseMouseCamFocus = mxMouseCamFocus;
}

VECT MouseCamGetMoveFromMouseDelta( float fScreenXDelta, float fScreenYDelta )
{
VECT	xMove;
VECT	xCamDir;
VECT	xCamRight;
VECT	xUp;

	xUp.x = 0.0f;
	xUp.y = 0.0f;
	xUp.z = 1.0f;

	xCamDir = *EngineCameraGetDirection();
	VectCross( &xCamRight, &xCamDir, &xUp );
	VectCross( &xUp, &xCamDir, &xCamRight );

	fScreenYDelta *= mfMoveSpeed * -1.0f;
	fScreenXDelta *= mfMoveSpeed;
	xMove.x = (fScreenYDelta * xUp.x) + (fScreenXDelta * xCamRight.x);
	xMove.y = (fScreenYDelta * xUp.y) + (fScreenXDelta * xCamRight.y);
	xMove.z = (fScreenYDelta * xUp.z) + (fScreenXDelta * xCamRight.z);
	return( xMove );
}

void MouseCamLeftMouseDownMove( float fScreenXDelta, float fScreenYDelta, float fTotalXDelta, float fTotalYDelta )
{
VECT		xMove;

	switch( mnMouseCamControlMode )
	{
	case CONTROL_MOVE:
		xMove = MouseCamGetMoveFromMouseDelta(fScreenXDelta, fScreenYDelta );
		VectAdd( &mxMouseCamPos, &mxMouseCamPos, &xMove );
/*
		mxMouseCamFocus.x = mxBaseMouseCamFocus.x + xMove.x;		
		mxMouseCamFocus.y = mxBaseMouseCamFocus.y + xMove.y;		
		mxMouseCamFocus.z = mxBaseMouseCamFocus.z + xMove.z;		
*/
		break;
	case CONTROL_ROT:
		mfViewAngleVert = mfOriginalViewAngleVert + (fTotalYDelta*mfRotSpeed);
		if ( mfViewAngleVert > TwoPi )
		{ 
			mfViewAngleVert -= TwoPi;
		}
		else if ( mfViewAngleVert < 0.0f )
		{
			mfViewAngleVert += TwoPi;
		}
		mfViewAngleHoriz = mfOriginalViewAngleHoriz + (fTotalXDelta*mfRotSpeed);
		if ( mfViewAngleHoriz > TwoPi )
		{ 
			mfViewAngleHoriz -= TwoPi;
		}
		else if ( mfViewAngleHoriz < 0.0f )
		{
			mfViewAngleHoriz += TwoPi;
		}
		break;
	case CONTROL_ZOOM:
		VectScale( &xMove, &mxMouseCamView, ((fScreenXDelta * mfZoomSpeed)*0.1f));
		VectAdd( &mxMouseCamPos, &mxMouseCamPos, &xMove );
	
/*
		mfMouseCamDist = mfOriginalCamDist + ((fScreenXDelta + fScreenYDelta) * mfZoomSpeed);
		if ( mfMouseCamDist < 0.01f )
		{
			mfMouseCamDist = 0.01f;
		}
*/
		break;
	}
	MouseCamUpdateCameraMatrices();
}

void MouseCamRightMouseDownMove( float fScreenXDelta, float fScreenYDelta, float fTotalXDelta, float fTotalYDelta )
{
}


int MouseCamWindowMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
POINTS points;

    switch( msg )
    {
    case WM_MOUSEMOVE:
		VECT	xMouseDelta;
		VECT	xMouseDeltaFrame;

		points = MAKEPOINTS(lParam);

		mxMousePos.x = points.x;
		mxMousePos.y = points.y;

		xMouseDelta.x = mxMousePos.x - mxMouseDownPos.x;
		xMouseDelta.y = mxMousePos.y - mxMouseDownPos.y;

		xMouseDeltaFrame.x = mxMousePos.x - mxMouseLastPos.x;
		xMouseDeltaFrame.y = mxMousePos.y - mxMouseLastPos.y;

		if ( mboRightMouseDown == TRUE )
		{
			MouseCamRightMouseDownMove( xMouseDeltaFrame.x, xMouseDeltaFrame.y, xMouseDelta.x, xMouseDelta.y );
		}
		else if ( mboLeftMouseDown == TRUE )
		{
			MouseCamLeftMouseDownMove( xMouseDeltaFrame.x, xMouseDeltaFrame.y, xMouseDelta.x, xMouseDelta.y );
		}
		else if ( mboMidMouseDown == TRUE )
		{
			MouseCamMidMouseDownMove( xMouseDeltaFrame.x, xMouseDeltaFrame.y, xMouseDelta.x, xMouseDelta.y );
		}
		mxMouseLastPos = mxMousePos;
		break;
	case WM_RBUTTONUP:
		mboRightMouseDown = FALSE;
		break;
	case WM_MBUTTONUP:
		mboMidMouseDown = FALSE;
		break;
	case WM_MBUTTONDOWN:
		mboMidMouseDown = TRUE;
		points = MAKEPOINTS(lParam);

		mxMouseDownPos.x = points.x;
		mxMouseDownPos.y = points.y;

		MouseCamMouseDownStore();
		break;
	case WM_RBUTTONDOWN:
		mboRightMouseDown = TRUE;
		points = MAKEPOINTS(lParam);

		mxMouseDownPos.x = points.x;
		mxMouseDownPos.y = points.y;

		MouseCamMouseDownStore();
		break;
	case WM_LBUTTONDOWN:
		mboLeftMouseDown = TRUE;
		points = MAKEPOINTS(lParam);

		mxMouseDownPos.x = points.x;
		mxMouseDownPos.y = points.y;
		mxMouseLastPos = mxMouseDownPos;

		MouseCamMouseDownStore();
		break;
	case WM_LBUTTONUP:
		mboLeftMouseDown = FALSE;
		break;

	}
	return( FALSE );
}
