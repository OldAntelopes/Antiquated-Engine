#ifndef LANDSCAPE_SEA_H
#define LANDSCAPE_SEA_H


extern void		SeaRenderInit( void );

extern float	SeaGetHeight( void );

extern void		SeaRenderAddTile( int nMapX, int nMapY );

extern void		SeaRenderFlush( void );

extern void		SeaRenderFree( void );

extern int		SeaGetTextureHandle( void );

#endif