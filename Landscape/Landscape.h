#ifndef GAMECOMMON_LANDSCAPE_H
#define GAMECOMMON_LANDSCAPE_H

#include "LandscapeCoords.h"
#include "LandscapeHeightmap.h"

enum eLandscapeTextureSystem
{
	LANDSCAPETEXTURES_BLANK,
	LANDSCAPETEXTURES_TILEGEN,
	LANDSCAPETEXTURES_MAP,
};



class LandscapeInitParams
{
public:
	LandscapeInitParams()
	{
		bUseHeightmap = TRUE;
		bUseNature = TRUE;
		bUseSea = TRUE;
		bUseMapData = TRUE;
		bAddToPhysicsWorld = TRUE;
		nHeightmapWidth = 256;
		nHeightmapHeight = 256;
		nLandscapeTextureSystem = LANDSCAPETEXTURES_TILEGEN;
		fTileSize = 10.0f;
		fTreeScaleMin = 0.8f;
		fTreeScaleMax = 1.5f;
		fTreeGenDensity = 1.0f;
		fLandHeightScale = 1.0f;
	}

	BOOL	bUseHeightmap;
	BOOL	bUseNature;
	BOOL	bUseSea;
	BOOL	bUseMapData;
	BOOL	bAddToPhysicsWorld;

	eLandscapeTextureSystem		nLandscapeTextureSystem;

	int		nHeightmapWidth;
	int		nHeightmapHeight;

	float	fTileSize;
	float	fLandHeightScale;

	float	fTreeScaleMin;
	float	fTreeScaleMax;
	float	fTreeGenDensity;

};


extern void			LandscapeInit( const char* szLevelRootFolder, LandscapeInitParams* pInitParams = NULL );
extern void			LandscapeLoadMap( void );

extern void			LandscapeUpdate( float fDelta );
extern void			LandscapeRender( void );
extern void			LandscapeRenderEnvironment( int nMode );
extern void			LandscapeShutdown( void );

extern void			LandscapePreGraphicsDeviceReset( void );
extern void			LandscapePostGraphicsDeviceReset( void );

extern void			LandscapeModifyHeight( int nMapX, int nMapY, float fNewHeightInWorldSpace );

extern void			LandscapeGetOrientationMatrix( VECT* pxPos, float fRadius, ENGINEMATRIX* pOutMatrix );
extern void			LandscapeGetYawPitchRoll( const VECT* pxPos, float fRotZ, float fRadius, VECT* pxRotOut );
extern VECT			LandscapeGetAlignedForwardVector( const VECT* pxPos, float fRotZ, float fRadius, VECT* pxRotOut );

enum eLANDTYPE
{
	LANDTYPE_SEA,
	LANDTYPE_BEACH,
	LANDTYPE_GRASS,
	LANDTYPE_ROCK,
	LANDTYPE_SNOW,
};

extern eLANDTYPE		LandscapeGetTileType( int nMapX, int nMapY );


//-----------------------------------------------------
//---- Debug

extern void						LandscapeShowDebug( BOOL bDebugOn );

//-----------------------------------------------------
//---- Landscape Internal

extern const char*				LandscapeGetLevelRootPath( void );

extern LandscapeInitParams*		LandscapeGetInitParams( void );


#endif
