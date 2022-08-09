

#include <StandardDef.h>		// This specifies a minimal set of 'standard' defines like BOOL
#include <Engine.h>				// The 3d graphics engine
#include <Rendering.h>

#include "Model.h"



Model::Model()
{
	m_ModelHandle = NOTFOUND;
}


Model::~Model()
{


}

//-------------------------------------------------------------------------
// Function    : Model::Load
// Description : 
//-------------------------------------------------------------------------
int		Model::Load( const char* szFilename )
{
	m_ModelHandle = ModelLoad( szFilename, 0, 1.0f );

	return( m_ModelHandle );
}


//-------------------------------------------------------------------------
// Function    : Model::Render
// Description : 
//-------------------------------------------------------------------------
void	Model::Render( void )
{
VECT	xPos = { 0.0f, 0.0f, 0.0f };
VECT	xRot = { 0.0f, 0.0f, 0.0f };

	// Lets set an anim going too for fun
	ModelSetAnimationNext( m_ModelHandle, ANIM_WALK, 0, NULL, 0 );

	ModelRender( m_ModelHandle, &xPos, &xRot, 0 );

}
