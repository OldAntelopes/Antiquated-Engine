#ifndef MAP_DATA_H
#define MAP_DATA_H

#include "MapDataPacketTypes.h"


extern void		MapDataSaveInit( void );

extern void		MapDataSavePacketWrite( int nPacketType, void* pPacket, int nSize );

extern void		MapDataSaveComplete( void );

#endif
