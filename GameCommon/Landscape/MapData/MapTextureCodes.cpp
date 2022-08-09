
#include "StandardDef.h"

#include "../LandscapeTextures.h"
#include "MapTextureCodes.h"

#define		NUMBER_OF_SHAPES_PER_SURFACE		16
#define		NUMBER_OF_SHAPES_PER_SECONDARY_SURFACE		8

enum		// Enum list of the texture pages dealt with by the landscape renderer
			//  Each tile on the map is assigned a wTextureNum (0-32767) describing the landscape tile
			//  e.g. wTextureNum = 0 to 7 is the main 'grass' texture
			//       wTextureNum = 8 to 23 are all 'surface 1'   etc etc
			// These values are defined in the --- Surface Type List in Island.h
			// This list is used to reference the different actual textures that are rendered. 
			//  i.e. wTextureNum = 8 to 23 -> Texture Page: TEXTURE_PAGE_SURFACE_1


{
	TEXTURE_PAGE_SURFACE_1 = 0,
	TEXTURE_PAGE_SURFACE_2,
	TEXTURE_PAGE_SURFACE_3,
	TEXTURE_PAGE_SURFACE_4, 
	TEXTURE_PAGE_SURFACE_5,
	TEXTURE_PAGE_SURFACE_6,
	TEXTURE_PAGE_SURFACE_7,
	TEXTURE_PAGE_SURFACE_8,
	TEXTURE_PAGE_SURFACE_9,
	TEXTURE_PAGE_SURFACE_10,
	TEXTURE_PAGE_SURFACE_11,
	TEXTURE_PAGE_SURFACE_12,		// 11

	TEXTURE_PAGE_COAST,
	TEXTURE_PAGE_SEA,
	TEXTURE_PAGE_UNUSED,		// 14		(Old Sea reflection)
	
	TEXTURE_PAGE_TMAP1,	// Tmap textures handled differently.. include these at the end of the list (before max_Landscape_textures)
	TEXTURE_PAGE_TMAP2,
	TEXTURE_PAGE_TMAP3,
	TEXTURE_PAGE_TMAP4,
	TEXTURE_PAGE_TMAP5,
	TEXTURE_PAGE_TMAP6,
	TEXTURE_PAGE_TMAP7,
	TEXTURE_PAGE_TMAP8,
	TEXTURE_PAGE_TMAP9,
	TEXTURE_PAGE_TMAP10,
	TEXTURE_PAGE_TMAP11,
	TEXTURE_PAGE_TMAP12,
	TEXTURE_PAGE_TMAP13,
	TEXTURE_PAGE_TMAP14,
	TEXTURE_PAGE_TMAP15,
	TEXTURE_PAGE_TMAP16,		// 30

	TEXTURE_PAGE_AUTOROAD1, 

	MAX_LANDSCAPE_TEXTURES = 64,
};


enum		// Shapes
{
	CURVE_ORIENT1 = 0,
	CURVE_ORIENT2,		// 1
	CURVE_ORIENT3,		// 2
	CURVE_ORIENT4,		// 3
	STRAIGHT_ROAD_ORIENT1,		// 4
	STRAIGHT_ROAD_ORIENT2,		// 5
	TJUNCTION_ORIENT1,			// 6
	TJUNCTION_ORIENT2,			// 7
	TJUNCTION_ORIENT3,			// 8
	TJUNCTION_ORIENT4,			// 9
	CROSSROADS,					// 10
	BLANK,						// 11
	ROTATED_EDGE,				// 12
	INVERSE_ROTATED_EDGE,		// 13
	LAYER_8,					// 14
	LAYER_9,					// 15

	END_LAYER_LIST = NUMBER_OF_SHAPES_PER_SURFACE,
};
//-------------------------------------------------------------------------------------
// ------- Surface Type List
//  Each tile on the map has a wTextureNum value (0 - 32767) that specifies the landscape to be drawn on it
//
//-------------------------------------------------------------------------------------
	// Tiles with wTextureNum 0 - 7 are the 'main' 'grass' surface, which is sometimes rendered by special magic
#define	SURFACE_TYPE_GRASS		0

	// Surfaces - these are tiles mapped to a single texture (i.e. whole texture is rendered on single tile)
#define	SURFACE_TYPE_1			8	// Beaten path
#define	SURFACE_TYPE_2			(SURFACE_TYPE_1+NUMBER_OF_SHAPES_PER_SURFACE)	// dirt road
#define	SURFACE_TYPE_3			(SURFACE_TYPE_2+NUMBER_OF_SHAPES_PER_SURFACE)	// Cobbled
#define	SURFACE_TYPE_4			(SURFACE_TYPE_3+NUMBER_OF_SHAPES_PER_SURFACE)	// Tarmac
#define	SURFACE_TYPE_5			(SURFACE_TYPE_4+NUMBER_OF_SHAPES_PER_SURFACE)	// Surface 5
#define	SURFACE_TYPE_6			(SURFACE_TYPE_5+NUMBER_OF_SHAPES_PER_SECONDARY_SURFACE)	// Surface 6
#define	SURFACE_TYPE_7			(SURFACE_TYPE_6+NUMBER_OF_SHAPES_PER_SECONDARY_SURFACE)	// Surface 7
#define	SURFACE_TYPE_8			(SURFACE_TYPE_7+NUMBER_OF_SHAPES_PER_SECONDARY_SURFACE)	// Surface 8
#define	SURFACE_TYPE_9			(SURFACE_TYPE_8+NUMBER_OF_SHAPES_PER_SECONDARY_SURFACE)	// Surface 9
#define	SURFACE_TYPE_10			(SURFACE_TYPE_9+NUMBER_OF_SHAPES_PER_SECONDARY_SURFACE)	// Surface 10
#define	SURFACE_TYPE_11			(SURFACE_TYPE_10+NUMBER_OF_SHAPES_PER_SECONDARY_SURFACE)	// Surface 11
#define	SURFACE_TYPE_12			(SURFACE_TYPE_11+NUMBER_OF_SHAPES_PER_SECONDARY_SURFACE)	// Surface 12

	// wTextureNum 8 -> 127 are the standard 1-tile surfaces

#define	MAX_STANDARD_SURFACE	(SURFACE_TYPE_12+NUMBER_OF_SHAPES_PER_SECONDARY_SURFACE)	// End of surface texture nums

	// wTextureNum 128 -> 131 denote types of texture map.. not really sure why.
#define	SURFACE_TYPE_TEXMAP14		(MAX_STANDARD_SURFACE+1)
#define	SURFACE_TYPE_TEXMAP58		(MAX_STANDARD_SURFACE+2)
#define	SURFACE_TYPE_TEXMAP912		(MAX_STANDARD_SURFACE+3)
#define	SURFACE_TYPE_TEXMAP1316		(MAX_STANDARD_SURFACE+4)
#define	SURFACE_TYPE_OTHER			(SURFACE_TYPE_TEXMAP1316+1)

#define	NUM_ROTATIONS_PER_SURFACE	3

	// wTextureNum 132 -> 168 are rotated versions of all the main surfaces
#define	SURFACE_TYPE_1_ROT		SURFACE_TYPE_OTHER 
#define	SURFACE_TYPE_2_ROT		(SURFACE_TYPE_1_ROT+NUM_ROTATIONS_PER_SURFACE)
#define	SURFACE_TYPE_3_ROT		(SURFACE_TYPE_2_ROT+NUM_ROTATIONS_PER_SURFACE)
#define	SURFACE_TYPE_4_ROT		(SURFACE_TYPE_3_ROT+NUM_ROTATIONS_PER_SURFACE)
#define	SURFACE_TYPE_5_ROT		(SURFACE_TYPE_4_ROT+NUM_ROTATIONS_PER_SURFACE)
#define	SURFACE_TYPE_6_ROT		(SURFACE_TYPE_5_ROT+NUM_ROTATIONS_PER_SURFACE)
#define	SURFACE_TYPE_7_ROT		(SURFACE_TYPE_6_ROT+NUM_ROTATIONS_PER_SURFACE)
#define	SURFACE_TYPE_8_ROT		(SURFACE_TYPE_7_ROT+NUM_ROTATIONS_PER_SURFACE)
#define	SURFACE_TYPE_9_ROT		(SURFACE_TYPE_8_ROT+NUM_ROTATIONS_PER_SURFACE)
#define	SURFACE_TYPE_10_ROT		(SURFACE_TYPE_9_ROT+NUM_ROTATIONS_PER_SURFACE)
#define	SURFACE_TYPE_11_ROT		(SURFACE_TYPE_10_ROT+NUM_ROTATIONS_PER_SURFACE)
#define	SURFACE_TYPE_12_ROT		(SURFACE_TYPE_11_ROT+NUM_ROTATIONS_PER_SURFACE)

#define	MAX_ROTATED_SURFACE		(SURFACE_TYPE_12_ROT+NUM_ROTATIONS_PER_SURFACE)	// End of rotated surface texture nums

	// wTextureNum 200 -> 256 are special codes

// Texture codes	(These are special cases for some types of surface that 
// are drawn differently or at a different point in the render
enum		
{
	TEXTURE_CODE_SEA_REFLECTION = 200,
	TEXTURE_CODE_SEA			= 201,
	TEXTURE_CODE_SAFE_ZONE		= 202,
	TEXTURE_CODE_TMAP_1			= 203,
	TEXTURE_CODE_TMAP_2			= 204,
	TEXTURE_CODE_TMAP_3			= 205,
	TEXTURE_CODE_TMAP_4			= 206,
	TEXTURE_CODE_TMAP_5			= 207,
	TEXTURE_CODE_TMAP_6			= 208,
	TEXTURE_CODE_TMAP_7			= 209,
	TEXTURE_CODE_TMAP_8			= 210,
	TEXTURE_CODE_TMAP_9			= 211,
	TEXTURE_CODE_TMAP_10		= 212,
	TEXTURE_CODE_TMAP_11		= 213,
	TEXTURE_CODE_TMAP_12		= 214,
	TEXTURE_CODE_TMAP_13		= 215,
	TEXTURE_CODE_TMAP_14		= 216,
	TEXTURE_CODE_TMAP_15		= 217,
	TEXTURE_CODE_TMAP_16		= 218,
		// ------------------------------------------
	TEXTURE_CODE_PITCH_1		= 219,
	TEXTURE_CODE_PITCH_2		= 220,
	TEXTURE_CODE_GOAL_1		= 221,
	TEXTURE_CODE_GOAL_2		= 222,
	TEXTURE_CODE_GOAL_3		= 223,
	TEXTURE_CODE_GOAL_4		= 224,
		// ------------------------------------------
	TEXTURE_CODE_AUTOROAD	= 225,
};

int		manTextureNumSurfaceLookup[256];
int		manTextureNumSurfacePageLookup[256];

//-------------------------------------------------------------------------------------


void		MapTextureCodesInit( void )
{
int	nTexture;

	for ( nTexture = 0; nTexture < 256; nTexture++ )
	{
		if ( nTexture < SURFACE_TYPE_1 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_GRASS;
		}
		else if ( nTexture < SURFACE_TYPE_2 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_1;
		}
		else if ( nTexture < SURFACE_TYPE_3 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_2;
		}
		else if ( nTexture < SURFACE_TYPE_4 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_3;
		}
		else if ( nTexture < SURFACE_TYPE_5 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_4;
		}
		else if ( nTexture < SURFACE_TYPE_6 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_5;
		}
		else if ( nTexture < SURFACE_TYPE_7 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_6;
		}
		else if ( nTexture < SURFACE_TYPE_8 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_7;
		}
		else if ( nTexture < SURFACE_TYPE_9 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_8;
		}
		else if ( nTexture < SURFACE_TYPE_10 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_9;
		}
		else if ( nTexture < SURFACE_TYPE_11 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_10;
		}
		else if ( nTexture < SURFACE_TYPE_12 )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_11;
		}
		else if ( nTexture < MAX_STANDARD_SURFACE )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_12;
		}
		else if ( nTexture < SURFACE_TYPE_2_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_1;
		}
		else if ( nTexture < SURFACE_TYPE_3_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_2;
		}
		else if ( nTexture < SURFACE_TYPE_4_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_3;
		}
		else if ( nTexture < SURFACE_TYPE_5_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_4;
		}
		else if ( nTexture < SURFACE_TYPE_6_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_5;
		}
		else if ( nTexture < SURFACE_TYPE_7_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_6;
		}
		else if ( nTexture < SURFACE_TYPE_8_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_7;
		}
		else if ( nTexture < SURFACE_TYPE_9_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_8;
		}
		else if ( nTexture < SURFACE_TYPE_10_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_9;
		}
		else if ( nTexture < SURFACE_TYPE_11_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_10;
		}
		else if ( nTexture < SURFACE_TYPE_12_ROT )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_11;
		}
		else if ( nTexture < MAX_ROTATED_SURFACE )
		{
			manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_12;
		}
		else // is a texmap (?)
		{
			switch ( nTexture )
			{
			case TEXTURE_CODE_TMAP_1:
			case TEXTURE_CODE_TMAP_2:
			case TEXTURE_CODE_TMAP_3:
			case TEXTURE_CODE_TMAP_4:
				manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_TEXMAP14;
				break;
			case TEXTURE_CODE_TMAP_5:
			case TEXTURE_CODE_TMAP_6:
			case TEXTURE_CODE_TMAP_7:
			case TEXTURE_CODE_TMAP_8:
				manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_TEXMAP58;
				break;
			case TEXTURE_CODE_TMAP_9:
			case TEXTURE_CODE_TMAP_10:
			case TEXTURE_CODE_TMAP_11:
			case TEXTURE_CODE_TMAP_12:
				manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_TEXMAP912;
				break;
			case TEXTURE_CODE_TMAP_13:
			case TEXTURE_CODE_TMAP_14:
			case TEXTURE_CODE_TMAP_15:
			case TEXTURE_CODE_TMAP_16:
				manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_TEXMAP1316;
				break;
			default:
				manTextureNumSurfaceLookup[nTexture] = SURFACE_TYPE_GRASS;
				break;
			}
		}

		switch( manTextureNumSurfaceLookup[nTexture] )
		{
		case SURFACE_TYPE_1:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_1;
			break;
		case SURFACE_TYPE_2:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_2;
			break;
		case SURFACE_TYPE_3:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_3;
			break;
		case SURFACE_TYPE_4:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_4;
			break;
		case SURFACE_TYPE_5:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_5;
			break;
		case SURFACE_TYPE_6:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_6;
			break;
		case SURFACE_TYPE_7:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_7;
			break;
		case SURFACE_TYPE_8:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_8;
			break;
		case SURFACE_TYPE_9:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_9;
			break;
		case SURFACE_TYPE_10:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_10;
			break;
		case SURFACE_TYPE_11:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_11;
			break;
		case SURFACE_TYPE_12:
			manTextureNumSurfacePageLookup[ nTexture ] = TEXTURE_PAGE_SURFACE_12;
			break;
		case SURFACE_TYPE_GRASS:
			manTextureNumSurfacePageLookup[ nTexture ] = MAX_LANDSCAPE_TEXTURES;
			break;
		}
	}

}


void		MapTextureCodesGetTypeAndIndex( int nTextureCode, int* pnTextureType, int* pnTextureIndex )
{
	*pnTextureType = 0;
	*pnTextureIndex = 0;

	if ( ( nTextureCode > 0 ) &&
		 ( nTextureCode < 256 ) )
	{
	int		nSurfacePage = manTextureNumSurfacePageLookup[ nTextureCode ];

		if ( ( nSurfacePage >= TEXTURE_PAGE_SURFACE_1 ) &&
			 ( nSurfacePage <= TEXTURE_PAGE_SURFACE_12 ) )
		{
			*pnTextureType = LANDSCAPE_TEXTURETYPE_TILE;
			*pnTextureIndex = nSurfacePage - TEXTURE_PAGE_SURFACE_1;
		}
	}
}
