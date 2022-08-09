#ifndef ENGINE_SKINNEDMESH_RENDER_H
#define	ENGINE_SKINNEDMESH_RENDER_H

#ifdef __cplusplus
extern "C"
{
#endif


extern HRESULT UpdateFrames(SFrame *pframeCur, D3DXMATRIX &matCur);
extern HRESULT DrawFrames(SFrame *pframeCur, UINT &cTriangles);




#ifdef __cplusplus
}
#endif


#endif
