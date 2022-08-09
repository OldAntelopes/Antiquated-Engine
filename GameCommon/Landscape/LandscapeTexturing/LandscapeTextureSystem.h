#ifndef GAMECOMMON_LANDSCAPE_LANDSCAPETEXTURESYSTEM_H
#define GAMECOMMON_LANDSCAPE_LANDSCAPETEXTURESYSTEM_H


class LandscapeTextureSystem
{
public:
	virtual ~LandscapeTextureSystem() {}

	// Initialise
	// - Called once on init.
	virtual void		Initialise( void ) = 0;

	// NewFrame
	// - Called once at the start of each game update frame
	virtual void		NewFrame( void ) = 0;

	// Shutdown
	// - Shutdown. Ya know
	virtual void		Shutdown( void ) = 0;

	// GetTextureNum
	// ------------ Gets the LandscapeTextureNum for a specified tile 
	//  (The landscapeTextureNum is used to group tiles and therefore usually equates to a texture used internally in the TextureSystem,
	//   but a landscapeTextureNum doesn't necessarily refer to a loaded texture )
	virtual int			GetTextureNum( int nMapX, int nMapY ) = 0;

	// ApplyTexture
	// ------------ Sets the appropriate texture for the current landscape texture #
	virtual void		ApplyTexture( int nLandscapeTextureNum, int nMapX, int nMapY ) = 0;

	// GetUVs
	// ------------ Get UV coords for a tile
	virtual void		GetUVs( int nLandscapeTextureNum, int nMapX, int nMapY, float* pfUBase, float* pfVBase, float* pfUWidth, float* pfVWidth ) = 0;

	// SetTextureResource
	// ------------ Called by gamecode to set a texture handle for a particular type/index (which may or not mean anything depending on the implementation - mostly for MapTextureSystem) 
	virtual void		SetTextureResource( int nTextureType, int nTextureTypeIndex, int hTexture ) {}

};








#endif