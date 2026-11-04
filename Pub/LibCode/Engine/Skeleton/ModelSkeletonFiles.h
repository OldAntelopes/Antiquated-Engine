#ifndef ENGINE_MODEL_SKELETON_FILES_H
#define ENGINE_MODEL_SKELETON_FILES_H

// .atm chunk export/load for skeleton data. Requires ModelFiles.h and ModelRendering.h to be included first

extern byte*	ModelSkeletonExportChunkBones( MODEL_RENDER_DATA* pxModelData, int* pnSize, int* pnElements );
extern byte*	ModelSkeletonExportChunkWeights( MODEL_RENDER_DATA* pxModelData, int* pnSize, int* pnElements );
extern byte*	ModelSkeletonExportChunkAnimLinks( MODEL_RENDER_DATA* pxModelData, int* pnSize, int* pnElements );

extern void		ModelSkeletonLoadChunkBones( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem );
extern void		ModelSkeletonLoadChunkWeights( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem );
extern void		ModelSkeletonLoadChunkAnimLinks( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem );

#endif
