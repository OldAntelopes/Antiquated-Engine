
#ifndef MODEL_EDITOR_SCENE_OBJECT_H
#define MODEL_EDITOR_SCENE_OBJECT_H


class CViewInterface
{
public:
	virtual void	GetScreenCoordForWorldCoord( VECT* pxWorldCoord, int* pnX, int* pnY ) = 0;
	virtual void	GetRayForScreenCoord( int nScreenX, int nScreenY, VECT* pxRayStart, VECT* pxRayDir ) = 0;
};

struct MAPPING_LIST
{
	unsigned int		nFaceNum;
	unsigned int		nVertIndex1;
	unsigned int		nVertIndex2;
	unsigned int		nVertIndex3;
	float	u1;
	float	v1;
	float	u2;
	float	v2;
	float	u3;
	float	v3;
	unsigned int		nFlags;
};

class CSceneObject
{
public:
	enum eRENDER_MODE
	{
		NORMAL = 0,
		NORMAL_WIREFRAME_FACES,
		NORMAL_WITH_VERTEX_POINTS,
		NORMAL_SHOWNORMALS,
		NORMAL_WIREFRAME_FACES_WITH_SHOWNORMALS,
		NORMAL_WIREFRAME_ONLY,
	};

	CSceneObject();
	~CSceneObject();
	void	ReleaseDX();
	void	InitDX();

	int		Render( eRENDER_MODE mode );

	void	SetModel( int nModelHandle );
	void	SetTexture( TEXTURE_HANDLE hTexture );
	void	SetPos( VECT* pxPos );
	void	SetRot( VECT* pxRot );

	void	Reset( void );

	//--------------------------------------------------------
	int					GetModelHandle( void ) { return mnModelHandle; }
	TEXTURE_HANDLE		GetTextureHandle( void ) { return mhTexture; }
	TEXTURE_HANDLE		GetTextureHandle( int materialAttrib );
	void				ReleaseTexture( void );

	//--------------------------------------------------------
	void	SelectAllVerticesInScreenRegion( int X, int Y, int W, int H );
	void	SelectAllFacesInScreenRegion( int X, int Y, int W, int H );
	void	SelectVertexAtScreenPoint( int X, int Y );
	void	SelectFaceAtScreenPoint( int X, int Y );
	void	SelectFacesByMaterial( int attribID );
	void	InvertSelection( void );
	void	SetSelectedFaces( byte* pbSelectedFaceList );
	//--------------------------------------------------------
	void	MoveSelected( VECT* pxMove );
	void	ApplyMaterialToSelectedFaces( int attribID );
	void	SubdivideSelectedFaces( void );
	void	FixInsideOutSelectedFaces( void );
	void	GetSelectedMappingList( MAPPING_LIST* );
	void	ApplyMappingListUVs( MAPPING_LIST*, int );
	void	DeleteSelectedFaces( void );
	void	CopySelectedFaces( EngineMesh* pxNewMesh );

	int		GetNumFacesSelected( void ) { return( mnNumFacesSelected ); }
	int		GetNumVerticesSelected( void ) { return( mnNumVerticesSelected ); }

	int		GetSelectedFace( int nIndex );

	int		GetSingleSelectedFace( void );
	int		GetSingleSelectedVertex( void );
	int		GetSelectionMaterial( void );
	int		GetFirstSelectedVertex( void );
	//--------------------------------------------------------
	void	SetViewInterface( CViewInterface* pViewInterface ) { mpViewInterface = pViewInterface; }
	//---------------------------
	void	EnableSelectedFaceHighlight( bool bFlag ) { mbSelectedFaceHighlight = bFlag; }

	void	OnModelChanged( BOOL bNewVerts );

	void	ToggleWireframe() { mbRenderWireframe = !mbRenderWireframe; }

	BYTE*	GetVertexSelections( void ) { return( mpbVertexSelections ); }
private:

	void	UpdateFaceMeshList( void );
	void	RenderVertexPoints( VECT* pxPos, VECT* pxRot );
	void	RenderPolys( VECT* pxPos, VECT* pxRot );
	void	RenderNormals( VECT* pxPos, VECT* pxRot );
	void	ResetFaceList();

	int						mnModelHandle;
	TEXTURE_HANDLE			mhTexture;
	VERTEX_BUFFER_HANDLE	mhFaceBuffer;
	CViewInterface*			mpViewInterface;

	BYTE*	mpbVertexSelections;
	BYTE*	mpbFaceSelections;

	int		mnNumFacesSelected;
	int		mnNumVerticesSelected;
	bool	mbSelectedFaceHighlight;
	bool	mbRenderWireframe;

	VECT	mxPos;
	VECT	mxRot;
};


#endif
