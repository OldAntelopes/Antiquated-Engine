
#ifndef USING_OPENGL
#ifndef TUD11

#include "DirectX/EngineDX.h"

#ifndef TUD9
#include "DirectX/SkinnedMeshDX.h"
#endif

#include <StandardDef.h>

#include "SkinnedModel.h"



//------------------------------------------------------------------------------------------------------------------------

SkinnedModel::SkinnedModel()
{

}


SkinnedModel::~SkinnedModel()
{


}


void*	SkinnedModel::GetBaseMesh() 
{ 
#ifdef TUD9
	return( NULL );
#else
	return( (void*)m_pDXSkinMesh->GetBaseMesh() ); 
#endif
}


bool	SkinnedModel::Load( const char* szFilename )
{
#ifdef TUD9
	return( false );
#else
	m_pDXSkinMesh = new SkinnedMeshDX;
	return( m_pDXSkinMesh->Load( szFilename ) );
#endif
}


void	SkinnedModel::Render( void )
{
#ifdef TUD9

#else
	m_pDXSkinMesh->Render();
#endif
}



#endif		// #ifndef TUD11
#endif		// #ifndef USING_OPENGL