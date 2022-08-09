#ifndef INTERFACE_INTERNALS_OPENGL_H
#define INTERFACE_INTERNALS_OPENGL_H

#include "TextureLoader\TextureLoader.h"

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif


extern void		InterfaceUnpackCol( unsigned int ulARGBCol, float* pfRed, float* pfGreen, float* pfBlue, float* pfAlpha );

typedef glTexture*		LPGRAPHICSTEXTURE;

extern int		mnInterfaceDrawX;
extern int		mnInterfaceDrawY;


#ifdef __cplusplus
}
#endif



#endif
