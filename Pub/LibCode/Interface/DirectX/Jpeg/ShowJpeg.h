#ifndef SHOW_JPEG_H
#define SHOW_JPEG_H


#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	

extern void ClearBufferedJpegs( void );
extern void	DrawBufferedJpegs( int layer );
extern void DrawPrimaryJpeg( void );

#ifdef __cplusplus
}
#endif


#endif