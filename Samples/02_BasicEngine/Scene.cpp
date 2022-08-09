
#include <StandardDef.h>		// This specifies a minimal set of 'standard' defines like BOOL
#include <Interface.h>			// The 2d graphics engine
#include <Engine.h>				// The 3d graphics engine

#include "Model.h"
#include "Scene.h"


Model*		m_pTestModel;

//-------------------------------------------------------------------------
// Function    : SceneUpdateCamera
// Description : 
//-------------------------------------------------------------------------
void		SceneUpdateCamera( void )
{
	EngineCameraSetPos( 5.0f, -2.0f, 1.5f );
	EngineCameraSetDirection( -5.0f, 2.0f, -0.5f );
	//-------------------------------------------- TEMP for testing
	// for tiny.x
//	EngineCameraSetPos( 0.0f, -500.0f, 300.0f );
//	EngineCameraSetDirection( 0.0f, 10.0f, 0.0f );
	// for SkinnedModel.x
//	EngineCameraSetPos( 500.0f, 0.0f, 0.0f );
//	EngineCameraSetDirection( -1.0f, 0.0f, 0.0f );
	// for miley.x
//	EngineCameraSetPos( 5000.0f, 0.0f, 0.0f );
//	EngineCameraSetDirection( -10.0f, 0.0f, 0.0f );
	//-------------------------------------------- TEMP for testing

	EngineCameraSetUpVect( 0.0f, 0.0f, 1.0f );
	EngineCameraSetProjection( PI/4,0.01f,10000.0f );

	EngineCameraUpdate();

	EngineDefaultState();
}


//-------------------------------------------------------------------------
// Function    : SceneInitialise
// Description : 
//-------------------------------------------------------------------------
void		SceneInitialise( void )
{
	// Create and load a test model
	m_pTestModel = new Model;

	//-------------------------------------------- TEMP for testing
//	m_pTestModel->Load( "Data\\Samples\\SkinnedModel.x" );
//	m_pTestModel->Load( "Data\\Samples\\miley.x" );
//	m_pTestModel->Load( "Data\\Samples\\shorty-walk.x" );
//	m_pTestModel->Load( "Data\\Samples\\fatdude.x" );
//	m_pTestModel->Load( "Data\\Samples\\tiny.x" );
	//-------------------------------------------- TEMP for testing
	m_pTestModel->Load( "Data\\Samples\\caveman1.atm" );

	// Set up the camera
	SceneUpdateCamera();

	// Add a light
	VECT	xLightDir = { -1.0f, 1.0f, -0.1f };
	EngineActivateSimpleLight( 0, 0, 0xFF404040, &xLightDir );
}





//-------------------------------------------------------------------------
// Function    : SceneUpdate
// Description : 
//-------------------------------------------------------------------------
void		SceneUpdate( void )
{
	SceneUpdateCamera();

	m_pTestModel->Render();
}





//-------------------------------------------------------------------------
// Function    : SceneFree
// Description : 
//-------------------------------------------------------------------------
void		SceneFree( void )
{
	// TODO - Free testmodel etc
	SAFE_DELETE( m_pTestModel );
}

