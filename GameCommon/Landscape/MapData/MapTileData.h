#ifndef MAP_TILE_DATA_H
#define MAP_TILE_DATA_H


typedef struct
{
	int		nTextureType;
	int		nTextureTypeIndex;
	float	fTextureU;
	float	fTextureV;
	float	fTextureUW;
	float	fTextureVH;

} TILE_TEXTURE;

extern void		MapTileDataInit( void );

extern void		MapTileDataRegisterHandlers( void );

extern void		MapTileDataFree( void );

//-----------------------------------------------------------------------------------

extern float	MapTileDataGetGrassLevel( int nMapX, int nMapY );

extern int		MapTileDataGetLakeID( int nMapX, int nMapY );

extern void		MapTileDataGetTexture( int nMapX, int nMapY, TILE_TEXTURE* pxOut );

//-----------------------------------------------------------------------------------

extern void		MapTileDataParseMapGen( void );

extern void		MapTileDataSetLakeID( int nMapX, int nMapY, int nLakeID );

#endif