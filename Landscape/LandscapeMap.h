#ifndef LANDSCAPE_MAP_H
#define	LANDSCAPE_MAP_H


typedef struct
{
	int		nLandscapeTextureNum;
	float	fLandHeight;

} LANDSCAPE_MAP_DATA;


extern void						LandscapeMapInit( void );

extern LANDSCAPE_MAP_DATA*		LandscapeMapGetData( int nMapX, int nMapY );

extern void						LandscapeMapShutdown( void );


#endif