#ifndef GAMECOMMON_RENDERUTIL_SPRITES3D_H
#define GAMECOMMON_RENDERUTIL_SPRITES3D_H

#include "RenderObject.h"

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

	void		Render( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eRenderFlags nRenderFlags );
	void		RenderRot( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eRenderFlags nRenderFlags );
	void		RenderRotSoftEdges( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eRenderFlags nRenderFlags );
	void		RenderRotSoftEdgesComplex( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eRenderFlags nRenderFlags );

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
	virtual const char* GetName() const { return "Sprite Group"; }

	void			Release();
	void			IncRef() { mnRefs++; }	

	void	AddSprite( const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags );
	void	AddSpriteRot( const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags, float fRotation );
	void	AddSpriteScaleZ( const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags, float fScaleZ );
	void	AddSpriteRotScaleXY( const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags, float fRotation, float fXYScaleFactor );

	void	SetTexture( int hTexture ) { SetTextureHandle( hTexture ); }

	SPRITE_GROUP		mhGroupNum;
	int					mLayer;
	float				mfGridScale;
	Sprite*				mpSpriteList = NULL;
	int					mnNumSpritesInList = 0;
	uint32				mulLastRenderTick = 0;
	SpriteGroup*		mpNext = NULL;
	int					mnRefs = 0;
private:

	void	AddSpriteToInternalList( Sprite* pSprite );

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

extern SPRITE_GROUP	 Sprites3DGetManagedGroupHandle( int nTextureHandle, float fGridScale, eRenderFlags nRenderFlags, int layer = 0 );

extern SpriteGroup*	 Sprites3DGetManagedGroup( SPRITE_GROUP hSpriteGroup );

extern SpriteGroup*	 Sprites3DGetGroup( int nTextureHandle, float fGridScale, eRenderFlags nRenderFlags, int layer );

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
