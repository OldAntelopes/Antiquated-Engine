
#ifndef ENGINE_SKINNEDMESH_H
#define	ENGINE_SKINNEDMESH_H

#include "SkinMeshLoaderDX.h"

class SkinnedMeshDX
{
public:
	SkinnedMeshDX();
	~SkinnedMeshDX();

	bool	Load( const char* szFilename );
	void	Render( void );

	ID3DXMesh*		GetBaseMesh() { return( m_pmcSelectedMesh->pMesh ); }

	SMeshContainer *m_pmcSelectedMesh;
    SFrame *m_pframeSelected;
    SDrawElement *m_pdeSelected;
	SDrawElement *m_pdeHead;
private:


};

#ifdef __cplusplus
extern "C"
{
#endif

extern METHOD  m_method;

extern LPD3DXMATRIXA16 m_pBoneMatrices;
extern DWORD m_maxBones;
extern DWORD m_dwIndexedVertexShader[4];

extern D3DCAPS8	m_d3dCaps;


#ifdef __cplusplus
}
#endif


#endif
