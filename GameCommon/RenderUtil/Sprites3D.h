#ifndef GAMECOMMON_RENDERUTIL_SPRITES3D_H
#define GAMECOMMON_RENDERUTIL_SPRITES3D_H

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif

enum eSpriteGroupRenderFlags
{
	kSpriteRender_Default = 0,
	kSpriteRender_Additive = 0x1,
	kSpriteRender_Orientation_Flat = 0x2,
	kSpriteRender_Rotated = 0x4,
	kSpriteRender_Orientation_XAxis = 0x8,
	kSpriteRender_ColourBlend = 0x10,
	kSpriteRender_Orientation_YAxis = 0x20,
};

typedef int		SPRITE_GROUP;

extern void Sprites3DInitialise( void );
extern void Sprites3DShutdown( void );

extern void Sprites3DInitialiseGraphicsDeviceResources( void );
extern void Sprites3DReleaseGraphicsDeviceResources( void );


extern SPRITE_GROUP	 Sprites3DGetGroup( int nTextureHandle, float fGridScale, eSpriteGroupRenderFlags nRenderFlags );

extern void	Sprites3DAddSprite( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, ulong ulCol, int nFrameNum, int nFlags );
extern void	Sprites3DAddSpriteRot( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, ulong ulCol, int nFrameNum, int nFlags, float fRotation );
extern void	Sprites3DAddSpriteScaleZ( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, ulong ulCol, int nFrameNum, int nFlags, float fScaleZ );

extern void	Sprites3DFreeGroup( SPRITE_GROUP hGroup );

extern void Sprites3DFlush( BOOL bUseZWrite );



#ifdef __cplusplus
}
#endif


#endif
