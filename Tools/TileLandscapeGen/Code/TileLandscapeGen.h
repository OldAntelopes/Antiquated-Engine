#ifndef TILE_LANDSCAPE_GEN_H
#define TILE_LANDSCAPE_GEN_H

#ifdef __cplusplus
extern "C"
{
#endif

extern void	MainOnPress( int X, int Y );
extern void	MainOnRelease( int X, int Y );

extern BOOL	MainInitialise( void );

extern void	MainUpdateFrame( void );

extern void	MainShutdown( void );

extern BOOL	MainProcessUpdate( void );

#ifdef __cplusplus
}
#endif

#endif