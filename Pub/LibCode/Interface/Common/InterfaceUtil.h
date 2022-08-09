#ifndef INTERFACE_UTIL_H
#define INTERFACE_UTIL_H


#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif


extern void		InterfaceUnpackCol( unsigned int ulARGBCol, float* pfRed, float* pfGreen, float* pfBlue, float* pfAlpha );

extern int		InterfaceGetTextureInternal( const char* szFilename, int nFlags, int nArchiveHandle );

#ifdef __cplusplus
}
#endif



#endif
