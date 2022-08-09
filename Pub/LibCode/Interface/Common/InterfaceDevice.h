#ifndef INTERFACE_DEVICE_H
#define INTERFACE_DEVICE_H


#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif



extern void		OnSetWindowSize( BOOL boFullScreen, int nWidth, int nHeight );
extern void		OnSetInitialSize( BOOL boFullScreen, int nFullScreenSizeX, int nFullScreenSizeY , BOOL boSmallFlag );





#ifdef __cplusplus
}
#endif



#endif
