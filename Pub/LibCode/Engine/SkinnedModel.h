
#ifndef ENGINE_SKINNED_MODEL_H
#define	ENGINE_SKINNED_MODEL_H



class	SkinnedMeshDX;
class	EngineMesh;

class SkinnedModel
{
public:
	SkinnedModel();
	~SkinnedModel();

	bool	Load( const char* szFilename );
	void	Render( void );

//	ID3DXMesh*		GetBaseMesh();
	void*		GetBaseMesh();

private:

	SkinnedMeshDX*		m_pDXSkinMesh;
};

#endif
