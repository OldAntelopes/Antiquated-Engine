#ifndef MAP_DATA_H
#define MAP_DATA_H

#include "MapDataPacketTypes.h"

typedef	void(*MapPacketHandler)( int nPacketType, void* pxMapPacket );

extern void		MapDataRegisterPacketHandler( int nMapPacketID, MapPacketHandler fnButtonHandler );

extern void		MapDataInit( void );

extern void		MapDataLoad( void );

extern void		MapDataFree( void );

extern float	MapDataGetScale( void );
extern int		MapDataGetMapSize( void );
extern int		MapDataGetCoordinateSystem( void );

extern VECT		MapDataConvertCoordinate( float fX, float fY );
extern void		MapDataConvertMapCoordinate( int* pnMapX, int* pnMapY );

#endif
