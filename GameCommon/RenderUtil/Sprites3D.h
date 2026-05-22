#ifndef GAMECOMMON_RENDERUTIL_SPRITES3D_H
#define GAMECOMMON_RENDERUTIL_SPRITES3D_H

#include "RenderObject.h"

enum eSpriteGroupRenderFlags
{
	kSpriteRender_Default = 0,
	kSpriteRender_Additive = 0x1,
	kSpriteRender_Orientation_Flat = 0x2,
	kSpriteRender_Rotated = 0x4,
	kSpriteRender_Orientation_XAxis = 0x8,
	kSpriteRender_ColourBlend = 0x10,
	kSpriteRender_Orientation_YAxis = 0x20,
	kSpriteRender_SoftEdges = 0x40,
	kSpriteRender_Subtractive = 0x80,
	kSpriteRender_SingleColTexAlpha = 0x100,
	kSpriteRender_IncAlpha = 0x200,
	kSpriteRender_CustomAspect = 0x400,
	kSpriteRender_DestInv = 0x800,
	kSpriteRender_DestAdd = 0x1000,
};

typedef int		SPRITE_GROUP;


#ifdef __cplusplus

class MultiVertexBuffers;

class Sprite
{
public:
	Sprite()
	{
		mfRot = 0.0f;
	}

	void		Render( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags );
	void		RenderRot( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags );
	void		RenderRotSoftEdges( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags );
	void		RenderRotSoftEdgesComplex( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags );

	VECT		mxPos;
	float		mfRot;
	float		mfScale;
	float		mfScaleZ;
	float		mfAspectRatio;
	int			mnFrameNum;
	uint32		mulCol;
	Sprite*		mpNext;
};

class SpriteGroup : public RenderObject
{
public:
	SpriteGroup()
	{
		mpSpriteList = NULL;
		mpNext = NULL;
		mhGroupNum = NOTFOUND;
	}

	virtual int		OnRender( void );

	SPRITE_GROUP		mhGroupNum;
	int					mhTexture;
	int					mLayer;
	float				mfGridScale;
	eSpriteGroupRenderFlags	mRenderFlags;
	Sprite*				mpSpriteList;
	SpriteGroup*		mpNext;
private:
	void		ApplyRenderFlags();
};

#endif  // #ifdef __cplusplus



#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif


extern void Sprites3DInitialise( void );
extern void Sprites3DShutdown( void );

extern void Sprites3DInitialiseGraphicsDeviceResources( void );
extern void Sprites3DReleaseGraphicsDeviceResources( void );


extern SPRITE_GROUP	 Sprites3DGetGroup( int nTextureHandle, float fGridScale, eSpriteGroupRenderFlags nRenderFlags, int layer = 0 );

extern void	Sprites3DAddSprite( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags );
extern void	Sprites3DAddSpriteRot( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags, float fRotation );
extern void	Sprites3DAddSpriteScaleZ( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags, float fScaleZ );
extern void	Sprites3DAddSpriteRotScaleXY( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags, float fRotation, float fXYScaleFactor );


extern void	Sprites3DFreeGroup( SPRITE_GROUP hGroup );

extern void Sprites3DFlush( BOOL bUseZWrite );
extern void Sprites3DFlushLayer( int nLayerNum, BOOL bUseZWrite );




#ifdef __cplusplus
}
#endif


#endif
