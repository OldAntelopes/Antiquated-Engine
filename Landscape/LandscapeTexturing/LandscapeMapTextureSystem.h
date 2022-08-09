#ifndef GAMECOMMON_LANDSCAPE_MAP_TEXTURESYSTEM_H
#define GAMECOMMON_LANDSCAPE_MAP_TEXTURESYSTEM_H

#include "LandscapeTextureSystem.h"

class MapTexturesTextureResourceList;

class LandscapeMapTextureSystem : public LandscapeTextureSystem
{
public:
		// Initialise
	// - Called once on init.
	void		Initialise( void );

	// NewFrame
	// - Called once at the start of each game update frame
	void		NewFrame( void );

	// Shutdown
	// - Shutdown. Ya know
	void		Shutdown( void );

	// GetTextureNum
	// ------------ Gets the LandscapeTextureNum for a specified tile 
	//  (The landscapeTextureNum is used to group tiles and therefore usually equates to a texture used internally in the TextureSystem)
	int			GetTextureNum( int nMapX, int nMapY );

	// ApplyTexture
	// ------------ Sets the appropriate texture for the current landscape texture #
	void		ApplyTexture( int nLandscapeTextureNum, int nMapX, int nMapY );

	// GetUVs
	// ------------ Get UV coords for a tile
	void		GetUVs( int nLandscapeTextureNum, int nMapX, int nMapY, float* pfUBase, float* pfVBase, float* pfUWidth, float* pfVWidth );
	

	void		SetTextureResource( int nTextureType, int nTextureTypeIndex, int hTexture );

private:
	MapTexturesTextureResourceList*		mpMapTexturesTextureResourceList;
	int									mnNextTextureNum;

};



#endif  // #ifndef GAMECOMMON_LANDSCAPE_TILEGEN_TEXTURESYSTEM_H
