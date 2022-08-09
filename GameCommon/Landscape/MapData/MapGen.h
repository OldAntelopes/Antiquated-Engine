#ifndef MAPDATA_MAP_GEN_H
#define MAPDATA_MAP_GEN_H


enum eMapGenType
{
	MAPGEN_CLIFF,
	MAPGEN_NEARCLIFF,
	MAPGEN_SEA,
	MAPGEN_FULLGRASS,
	MAPGEN_PARTIALGRASS,
	MAPGEN_SAND,
	MAPGEN_ROCK,
	MAPGEN_SNOW,
	MAPGEN_EMPTY,
};

extern void		MapGenLoadBitmap( void );

extern eMapGenType		MapGenGetTileType( float fWorldX, float fWorldY );

extern void				MapGenReleaseBitmap( void );

extern void		MapGenSRand(int x);
extern int		MapGenRand( int nMax );
extern float	MapGenFRand( float fLow, float fHigh );


#endif
