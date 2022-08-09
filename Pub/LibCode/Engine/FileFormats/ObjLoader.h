#ifndef ENGINE_OBJ_LOADER_H
#define ENGINE_OBJ_LOADER_H





#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif


extern BOOL	ModelLoadOBJFile( MODEL_RENDER_DATA* pxModelData, const char* szFilename, byte* pbMem );


#ifdef __cplusplus
}
#endif







#endif