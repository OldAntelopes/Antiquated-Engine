#ifndef ENTITY_GRAPHICS_H
#define	ENTITY_GRAPHICS_H


extern int		EntityGraphicsCreate( const char* szModelNum, const char* szTextureNum, int nInstanceNum );

extern int		EntityGraphicsGetModelHandle( int nEntityGraphicNum );
extern int		EntityGraphicsGetTextureHandle( int nEntityGraphicNum );

extern void		EntityGraphicsDeleteAll( void );








#endif