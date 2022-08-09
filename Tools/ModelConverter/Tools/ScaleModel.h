#ifndef TOOLS_SCALE_MODEL_H
#define TOOLS_SCALE_MODEL_H


extern void			ScaleDialogActivate( void );

extern void			ModelConvScaleModelFree( int nModelHandle, float fX, float fY, float fZ );

extern void			ModelConvFixBuildingScale( int nHandle );

extern void			ModelConvScaleToTileSize( int nMode );
extern void			ModelConvScaleTo15mGrid( int nHandle );

#endif