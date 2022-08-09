#ifndef GAMECOMMON_RENDERUTIL_MODEL_SCREENSHOT_H
#define GAMECOMMON_RENDERUTIL_MODEL_SCREENSHOT_H


extern void		ModelScreenShotSetCamera( const VECT* pxCamPos, const VECT* pxCamDir, const VECT* pxCamUp );
extern int		ModelScreenShotCreate( int hModel, int hTexture, int nTextureSizeX, int nTextureSizeY, int nFlags );




#endif