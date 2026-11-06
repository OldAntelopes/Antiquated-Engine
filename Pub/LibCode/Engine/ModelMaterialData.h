#ifndef MODEL_MATERIALDATA_H
#define MODEL_MATERIALDATA_H

#include "ModelRendering.h"

typedef struct
{
	float	fRed;
	float	fGreen;
	float	fBlue;
	float	fAlpha;
} MATERIAL_COLOUR;				// Needs to match ENGINEMATERIAL_COLOUR

enum eTEX_CHANNELS
{
	DIFFUSEMAP,
	NORMALMAP,
	SPECULARMAP,

	MAX_NUM_TEX_CHANNELS
};

class ModelMaterialData
{
public:
	enum eBLEND_TYPES
	{
		NONE,
		ALPHABLEND,
		ADDITIVE_ALPHA,
		SUBTRACTIVE_ALPHA,
		COLOURBLEND,
		ADDITIVE_COLOUR,
		SUBTRACTIVE_COLOUR,
		ALPHABLEND_NOCUTOFF,

		NUM_BLEND_TYPES
	};

	enum eCOLOUR_CHANNELS
	{
		DIFFUSE,
		SPECULAR,
		EMISSIVE,
		AMBIENT,

		NUM_COLOUR_CHANNELS,
	};


	enum		// results from Apply
	{
		NO_CHANGES = 0,
		BLEND_RENDER_STATES_CHANGED = 0x1,
		MATERIAL_CHANGED = 0x2,
		SPECULAR_ACTIVATED = 0x4,
		NORMALMAP_ACTIVATED = 0x8,
		SPECULARMAP_ACTIVATED = 0x10,
		DOUBLESIDED_ACTIVATED = 0x20,
		RELIEF_ACTIVATED = 0x40,
	};

	typedef struct
	{
		char*			mszFilename;
		TEXTURE_HANDLE	mhTexture;
		int				m_nSizeOfSourceData;
		byte*			m_pSourceData;
		bool			m_bAllowOverrideTexture;
		bool			m_bEmbeddedTexture;

	} TEXCHANNEL;

	ModelMaterialData();
	~ModelMaterialData();

	void	LoadTextureFilename( int texNum, const char* szFilename );
	void	LoadTextureFromMem( int texNum, byte* pbMem, int nMemSize, const char* szOriginalFilename = NULL );

	void	SetAttrib( int nAttrib ) { m_nAttrib = nAttrib; }
	int		GetAttrib( void ) { return( m_nAttrib ); }

	bool	IsDoubledSided( void ) { return( m_bDoubleSided ); }
	void	SetDoubleSided( bool bFlag ) { m_bDoubleSided = bFlag; }

	void	SetNext( ModelMaterialData* pNext ) { m_pNext = pNext; }
	ModelMaterialData*		GetNext( void ) { return( m_pNext ); }

	int		Apply( void );

	void			SetFilename( int texnum, const char* szFilename );
	const char*		GetFilename( int texnum ) { return( m_aTexChannels[texnum].mszFilename ); }
	void			RemoveTexture( int texNum );

	bool			IsEmbeddedTexture( int texnum ) { return( m_aTexChannels[texnum].m_bEmbeddedTexture ); }
	void			SetIsEmbeddedTexture( int texnum, bool bFlag ) { m_aTexChannels[texnum].m_bEmbeddedTexture = bFlag; }

	void			SetSourceData( int texchannel, byte* pbData, int nDataSize );
	byte*			GetSourceData( int texchannel, int* pnSize );

	MATERIAL_COLOUR	GetColour( eCOLOUR_CHANNELS );
	void			SetColour( eCOLOUR_CHANNELS, const MATERIAL_COLOUR* pCol );

	float			GetSpecularPower() { return( m_specularPower ); }
	void			SetSpecularPower( float fVal ) { m_specularPower = fVal; }

	void			SetBlendType( eBLEND_TYPES type ) { m_BlendType = type; }
	eBLEND_TYPES	GetBlendType( void ) { return( m_BlendType ); }

	bool			HasActiveMaterialProperties( void ) { return( m_bMaterialPropertiesActive ); }
	void			SetActiveMaterialProperties( bool bFlag ) { m_bMaterialPropertiesActive = bFlag; }

	// The active material properties only apply at night (EngineGameInterface::IsDaytime). For lit windows and lamps.
	bool			IsMaterialPropertiesNightOnly( void ) { return( m_bMaterialPropertiesNightOnly ); }
	void			SetMaterialPropertiesNightOnly( bool bFlag ) { m_bMaterialPropertiesNightOnly = bFlag; }

	// A height map for parallax and surface lighting (ATMFormat.md, chunk 0x1F; EngineSetModelRelief)
	void			LoadReliefFromMem( byte* pbMem, int nMemSize, float fDepth, int nTextureWidth, int nTextureHeight );
	bool			HasRelief( void ) { return( m_hReliefTexture != NOTFOUND ); }
	float			GetReliefDepth( void ) { return( m_fReliefDepth ); }
	int				GetReliefTextureWidth( void ) { return( m_nReliefTextureWidth ); }
	int				GetReliefTextureHeight( void ) { return( m_nReliefTextureHeight ); }
	// Tools keep the file data, so they can save it again
	void			SetReliefSource( byte* pbMem, int nMemSize );
	byte*			GetReliefSource( int* pnSize ) { *pnSize = m_nReliefSourceSize; return( m_pReliefSource ); }

	TEXTURE_HANDLE	GetTexture( int texnum ) { return( m_aTexChannels[texnum].mhTexture ); }

	int				GetCloneTextureMaterialIndex( int texnum ) { return( m_nCloneTextureMaterial ); }
	void			CloneTextureFromMaterial( int texnum, ModelMaterialData* pSource );

	void			SetMaterialName( const char* szMaterialName );
	const char*		GetMaterialName( void ) { return( m_szMaterialName ); }

private:
	bool			m_bMaterialPropertiesActive;
	bool			m_bMaterialPropertiesNightOnly;

	ModelMaterialData*		m_pNext;
	int		m_nAttrib;
	eBLEND_TYPES		m_BlendType;

	char*			m_szMaterialName;

	TEXCHANNEL		m_aTexChannels[MAX_NUM_TEX_CHANNELS];

	int				m_nCloneTextureMaterial;

	MATERIAL_COLOUR		m_aColours[NUM_COLOUR_CHANNELS];
	float				m_specularPower;
	bool				m_bDoubleSided;

	TEXTURE_HANDLE		m_hReliefTexture;
	float				m_fReliefDepth;
	int					m_nReliefTextureWidth;
	int					m_nReliefTextureHeight;
	byte*				m_pReliefSource;
	int					m_nReliefSourceSize;
};

extern ModelMaterialData*		FindMaterialFromHandle( int nModelHandle, int nAttrib );
extern ModelMaterialData*		FindMaterial( MODEL_RENDER_DATA* pModel, int nAttrib );
extern ModelMaterialData*		FindMaterialByMaterialName( MODEL_RENDER_DATA* pModel, const char* szMaterialName );
extern ModelMaterialData*	ModelAddNewMaterialData( MODEL_RENDER_DATA* pModel, const char* acTexture );
extern ModelMaterialData*	ModelCreateDefaultMaterialsData( MODEL_RENDER_DATA* pModel, const char* acTexture );
extern void		ModelMaterialsLoadShader( void );
extern void		ModelMaterialsFreeShader( void );

extern void		ModelMaterialsDeactivateNormalShader( void );
extern void		ModelMaterialsActivateNormalShader( BOOL bHasDiffuseMap, BOOL bHasNormalMap, BOOL bHasSpecularMap );
extern void		ModelMaterialsNormalShaderUpdateWorldTransform( const ENGINEMATRIX* pMatWorld );
extern void		ModelMaterialsNormalShaderUpdateZBias( float fZBias );
extern void		ModelMaterialsShaderEnableOverlay( BOOL bHasOverlay );
// Sets a float4 constant by name on the model material pixel shader, if the shader declares it (otherwise nothing
// happens). For app-specific additions to an app's own ATMMaterial.psh, e.g. The Universal's landscape materials.
extern void		ModelMaterialsShaderSetFloat4( const char* szConstantName, const float* pafValues );
// The same for the model material vertex shader (ATMMaterial.vsh), e.g. The Universal's tree wind sway.
extern void		ModelMaterialsVertexShaderSetFloat4( const char* szConstantName, const float* pafValues );

extern void		ModelMaterialShaderSetMaterialProperties( MATERIAL_COLOUR* pxDiffuse, MATERIAL_COLOUR* pxAmbient, MATERIAL_COLOUR* pxSpecular, MATERIAL_COLOUR* pxEmissive, float fSpecularPower );
extern void		ModelMaterialsEnableLighting( BOOL bLightingEnable );
// Puts things back after a draw with a relief material (Apply returned RELIEF_ACTIVATED)
extern void		ModelMaterialsEndRelief( void );

#ifdef TOOL
extern void	ModelDeleteAllMaterials( int nModelHandle );
extern void	ModelCreateDefaultMaterials( int nModelHandle, const char* acTexture );
extern void	ModelAddNewMaterial( int nModelHandle, const char* acTexture );


extern void	ModelMaterialOptimiseTextureClones( MODEL_RENDER_DATA* pModel);
extern void	MaterialBrowserRemoveDiffuseTexture( void );

#endif

#endif // #ifndef MODEL_MATERIALDATA_H
