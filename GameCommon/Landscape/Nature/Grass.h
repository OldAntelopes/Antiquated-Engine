#ifndef NATURE_GRASS_H
#define NATURE_GRASS_H


typedef struct
{
	int		nMapX;
	int		nMapY;

	float	fDensity;

} GRASS_MAP_PACKET;



extern void		GrassInit( void );

extern void		GrassUpdate( float fDelta );

extern void		GrassRender( void );

extern void		GrassFree( void );

extern void		GrassAddRenderTile( int nMapX, int nMapY, float fGrassLevel );








#endif