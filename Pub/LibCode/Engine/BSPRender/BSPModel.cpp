
#include <stdio.h>

#include <standarddef.h>

#include "BSPModel.h"


struct quake2_bsp_lump
{   
	uint32    offset;     // offset (in bytes) of the data from the beginning of the file
    uint32    length;     // length (in bytes) of the data

};

struct quake2_bsp_header
{ 
	uint32    magic;      // magic number ("IBSP")
    uint32    version;    // version of the BSP format (38)

    quake2_bsp_lump  lump[19];   // directory of the lumps
};

/*
struct hlsource_dheader_t
{
      int         ident;            // BSP file identifier
      int         version;          // BSP file version
      hlsource_lump_t      lumps[HEADER_LUMPS];    // lump directory array
      int         mapRevision;      // the map's revision (iteration, version) number 
};
*/



void	BSPModel::Load( const char* szFilename )
{
FILE*	pFile;

	pFile = fopen( szFilename, "rb" );
	if ( pFile )
	{
	quake2_bsp_header	xHeader;
	char*	pcBytes = (char*)( &xHeader );

		fread( &xHeader, sizeof( xHeader ), 1, pFile );
		// if this is the correct version
		if ( xHeader.version == 38 )
		{
			// we have
		}
		fclose( pFile );
	}
}



void	BSPModel::Render( void )
{


}