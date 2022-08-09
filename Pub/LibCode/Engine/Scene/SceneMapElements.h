

#ifndef ISLAND_Scene_MAP_ELEMENTS_H
#define ISLAND_Scene_MAP_ELEMENTS_H


typedef	void(*ListFileReferencesCallback)( const char* szFilename, int nFilesize );


class SceneMapElement
{
friend class SceneMap;
public:
	typedef enum
	{
		ROOM,
		FURNITURE,
		EFFECT,
		ACTION,
		REGION,
		LIGHT,

	} ELEMENT_TYPES;

	SceneMapElement() 
	{ 
		mTextureHandle = 0; mModelHandle = NOTFOUND; mpChildElement = NULL; mpBrotherElement = NULL;
		mPos.x = mPos.y = mPos.z = 0.0f;
		mRot.x = mRot.y = mRot.z = 0.0f;
		mbIsSelected = FALSE;
#ifdef TOOL
		m_szTextureFilename[0] = 0;
		m_szModelFilename[0] = 0;
#endif
	}
	virtual ~SceneMapElement();
	void	SetModel( const char* szModelName );
	void	SetTexture( const char* szTextureName );
	
	virtual void 	Display( const VECT* pxPos, const VECT* pxRot, int nFlags );
	virtual void	OnParseComplete( void ) {}
	virtual void	OnScaleChange( void ) {}

	SceneMapElement*		GetChild() { return( mpChildElement ); }
	SceneMapElement*		GetBrother() { return( mpBrotherElement ); }
	void	Update( const VECT* pxPos );
	virtual void	UpdateForPlayer( const VECT* pxPos ) {}
	// for map editing etc..
	void	SetSelected( BOOL bFlag );
	BOOL	IsSelected( void ) { return( mbIsSelected ); }

	VECT*	GetPos( void ) { return( &mPos ); }
	VECT*	GetRot( void ) { return( &mRot ); }
	float	GetScale( void ) { return( m_fRadius ); }
	void	SetPos( VECT* pPos ) { mPos = *pPos; }
	void	SetRot( VECT* pRot ) { mRot = *pRot; }
	void	SetScale( float fRadius ) { m_fRadius = fRadius; }
	
	void	SetYaw( float fRotZ ) { mRot.z = ((fRotZ * TwoPi)/360.0f); }
	virtual const char*	GetTypeName( void ) { return( "Scene Root" ); }
#ifdef TOOL
	const char*		GetModelFilename( void ){ return( m_szModelFilename ); }
	const char*		GetTextureFilename( void ){ return( m_szTextureFilename ); }
#endif
	BOOL		IsModelLoaded( void ) { if ( mModelHandle == NOTFOUND ) return( FALSE ); else return( TRUE ); }
	BOOL		IsTextureLoaded( void ) { if ( mTextureHandle == 0 ) return( FALSE ); else return( TRUE ); }

	void		SetModelHandle( int nHandle ) { mModelHandle = nHandle; }
	int			GetModelHandle( void ) { return( mModelHandle ); }
protected:
	BOOL	Raycast( VECT* pxPos1, VECT* pxPos2, VECT* pxHit, VECT* pxNormal );
	SceneMapElement*		FindRegion( const char* szRegionType, const char* szRegionParam );
	SceneMapElement*		FindParentOf( SceneMapElement* pChild );

	SceneMapElement*		mpChildElement;
	SceneMapElement*		mpBrotherElement;
	VECT					mCombinedPos;
	VECT					mCombinedRot;
	VECT					mPos;
	VECT					mRot;
	float					m_fRadius;
	BOOL					mbIsSelected;
#ifdef TOOL
	void	Save( FILE* );
	void	GetFileReferences( ListFileReferencesCallback fnCallback );
	void	ListFileReferences( ListFileReferencesCallback fnCallback );

	virtual void	SaveElement( FILE* ) {}
	char		m_szModelFilename[256];
	char		m_szTextureFilename[256];
#endif
private:
	int					mModelHandle;
	int					mTextureHandle;

	static		int		msnNumSelected;

};

class SceneRoomElement : public SceneMapElement
{
public:
	const char*	GetTypeName( void ) { return( "Room" ); }

	virtual void	OnParseComplete( void );
protected:
#ifdef TOOL
	virtual void	SaveElement( FILE* );
#endif
};


class SceneFurnitureElement : public SceneMapElement
{
public:
	const char*	GetTypeName( void ) { return( "Furniture" ); }
protected:
#ifdef TOOL
	virtual void	SaveElement( FILE* );
#endif
};


class SceneEffectElement : public SceneMapElement
{
public:
	const char*	GetTypeName( void ) { return( "Effect" ); }
protected:
#ifdef TOOL
	virtual void	SaveElement( FILE* );
#endif
};

class SceneActionElement : public SceneMapElement
{
public:
	const char*	GetTypeName( void ) { return( "Action" ); }
protected:
#ifdef TOOL
	virtual void	SaveElement( FILE* );
#endif
};

class SceneLightElement : public SceneMapElement
{
public:
	const char*	GetTypeName( void ) { return( "Light" ); }

	void	SetColour( ulong ulARGB );
	void	SetLightType( int type );
	void	SetRange( float fScale );
	void	SetAttenuation( float fAtten );
	void	SetDirection( VECT* pxDir );
protected:
#ifdef TOOL
	virtual void	SaveElement( FILE* );
#endif
	ulong		mulLightColour;
	int			mnLightType;
	float		mfScale;
	float		mfAtten1;

};

class SceneRegionElement : public SceneMapElement
{
public:
	SceneRegionElement();
	~SceneRegionElement();

	const char*	GetTypeName( void ) { return( "Region" ); }

	char*	GetRegionType( void ) { return( m_szRegionType ); }
	char*	GetRegionParam( void ) { return( m_szRegionParam ); }

	void	SetRegionType( const char* szRegionType );
	void	SetRegionParam( const char* szRegionParam );

	virtual void	UpdateForPlayer( const VECT* pxPos );
	virtual void 	Display( const VECT* pxPos, const VECT* pxRot, int nFlags );
protected:
#ifdef TOOL
	virtual void	SaveElement( FILE* );
#endif
	char*		m_szRegionType;
	char*		m_szRegionParam;

	BOOL		m_bPlayerInRegion;
};

#endif