#ifndef GAMECOMMON_RIBBON_TRAIL_H
#define GAMECOMMON_RIBBON_TRAIL_H

#include "StandardDef.h"
#include "Engine.h"
#include "RenderObject.h"

//----------------------------------------------------------
// RibbonTrail
#define	MAX_POINTS_IN_TRAIL_LIST	512

#define	MAX_TRAIL_VERTICES		((MAX_POINTS_IN_TRAIL_LIST*2)+2)
#define	NUM_POLYS_IN_TRAIL		(((MAX_POINTS_IN_TRAIL_LIST-1)*2)+2)
#define	NUM_TRAIL_INDICES		(NUM_POLYS_IN_TRAIL*3)


class RibbonTrail : public RenderObject
{
public:
	typedef struct
	{
		VECT	xPos;
		VECT	xTangent;
		uint32	ulTimeAdded;
		BOOL	mbIsVisible;
		uint32	ulTintCol;
	} TRAIL_POINT;

	RibbonTrail();
	virtual ~RibbonTrail();

	virtual int		OnRender( void );


	void	Initialise( int nType );

	void	Update( const VECT* pxCurrentPos, uint32 ulPointGap, BOOL bDoDraw = TRUE );

	void	SetScale( float fScale ) { mfScale = fScale; }
	void	SetAlpha( float fAlpha ) { mfAlpha = fAlpha; }
	void	SetTint( uint32 ulCol ) { mulTintCol = ulCol; }
	void	SetDecayTime( uint32 ulTime ) { mulFadeHoldTimeMS = ulTime / 2; mulFadeOutTimeMS = ulTime / 2; }
	void	SetFadeProp( uint32 ulFadeHoldTime, uint32 ulFadeOutTime ) { mulFadeHoldTimeMS = ulFadeHoldTime; mulFadeOutTimeMS = ulFadeOutTime; }
	void	RequestDelete( BOOL bImmediately ) { mbWantsDelete = TRUE; mbDeleteImmediately = bImmediately; }

	BOOL	WantsDelete( void ) { return( mbWantsDelete ); }
	BOOL	IsAlive( void ) { return( mbIsAlive ); }
	BOOL	WantsImmediateDelete( void ) { return( mbDeleteImmediately ); }

	VECT	GetTangent( ) { return( mxLastValidTangent); }
	VECT	GetSmoothedTangent( );

	void	Reset( const VECT* pxIn );
private:


	uint32	GetColour( int nIndex );

	void	AddPos( const VECT* pxIn, BOOL bDoDraw = TRUE );
	BOOL	GetPos( int nIndex, VECT* pxOut, VECT* pxTangent );

	void	InitTrailBuffers( void );
	void	FreeVertexBuffer(void);
	void	CalcTrailUVs( int nType );
	void	UpdateInternal( void );
	void	AddMidPoint( const VECT* pxIn, uint32 ulTime, int nRecurseLayer, BOOL bIsVisible );
	void	AddPosImmediate( const VECT* pxIn, const VECT* pxTangent, uint32 ulTime, BOOL bIsVisible );

	TRAIL_POINT		maxTrailListInternal[MAX_POINTS_IN_TRAIL_LIST];
	int		mnNextTrailPoint;
	int		mnLastTrailPoint;
	uint32	mulLastInternalAddTick;
	int		mnType;
	VECT	mxCurrentPos;
	VECT	mxLastValidTangent;
	VECT	mxPrevValidTangent;
	VECT	mxLastValidRight;
	float	mfScale;
	float	mfAlpha;
	uint32	mulFadeHoldTimeMS;
	uint32	mulFadeOutTimeMS;
	uint32	mulTintCol;

	int		mhTrailVertexBuffer;
	int		mhTrailIndexBuffer;

	BOOL			mbWantsDelete;
	BOOL			mbDeleteImmediately;
	BOOL			mbIsAlive;
};


#endif