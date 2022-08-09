#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef __cplusplus
extern "C"
{
#endif


extern void		PlatformInit( void );

extern void		PlatformUpdateFrame( void );

extern void		PlatformGetCurrentHoldPosition( int* pnX, int* pnY );

extern int		PlatformSaveDataFileOpen( int slot, BOOL bWrite );
extern int		PlatformSaveDataFileWrite( unsigned char* pucData, int nMemSize );
extern int		PlatformSaveDataFileRead( unsigned char* pucData, int nMemSize );
extern int		PlatformSaveDataFileClose( void );
	
#ifdef __cplusplus
}
#endif


#endif


