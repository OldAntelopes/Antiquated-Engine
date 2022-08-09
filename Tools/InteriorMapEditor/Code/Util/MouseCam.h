#ifndef MOUSE_CAM_H
#define MOUSE_CAM_H

#ifdef __cplusplus
extern "C"
{
#endif

// A generic module for having a camera controlled by the mouse
// (i.e the one used in the model converter and interior map editor)

enum
{
	CONTROL_MOVE = 0,
	CONTROL_ROT,
	CONTROL_ZOOM,
};

enum
{
	VIEW_MODE_NORMAL = 0,
	VIEW_MODE_TOPDOWN,
	VIEW_MODE_FIRST_PERSON,
};

extern int MouseCamWindowMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

extern void	MouseCamSetControlMode( int Mode );
extern int	MouseCamGetControlMode( void );

extern void	MouseCamSetViewMode( int Mode );
extern void	MouseCamSetMoveSpeed( float fSpeed );
extern void	MouseCamSetRotSpeed( float fSpeed );
extern void	MouseCamSetZoomSpeed( float fSpeed );

extern VECT MouseCamGetMoveFromMouseDelta( float fScreenXDelta, float fScreenYDelta );

extern void	MouseCamSet( VECT* pPos, VECT* pFocus );
extern void MouseCamUpdateCameraMatrices( void );


#ifdef __cplusplus
}
#endif



#endif // MOUSE_CAM_H