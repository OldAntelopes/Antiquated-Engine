#ifndef MODEL_CONVERTER_FBX_LOADER_H
#define MODEL_CONVERTER_FBX_LOADER_H

//---------------------------------------------------------------------- External interface

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif


int			ModelLoadFBXFile( MODEL_RENDER_DATA* pxModelData, const char* szFilename, byte* pbMem );

// extern void		ModelLoaderExportFBX( int nModelHandle, const char* acFilename );				(Defined in ModelFiles.h)

#ifdef __cplusplus
}
#endif



#endif
