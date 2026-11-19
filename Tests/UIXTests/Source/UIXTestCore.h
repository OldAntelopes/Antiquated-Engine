#ifndef UIX_TEST_CORE_H
#define UIX_TEST_CORE_H

//------------------------------------------------------------------------------------------------
// UIXTestCore.h
//
// The harness's test framework. A test is a function registered with UIX_TEST. It builds UIX pages in
// code, drives them with scripted input, and checks three ways:
//
//  - UIX_CHECK( cond, "Area: what it shows" )  : a plain assertion, for behaviour we intend
//  - UIXTestLog / UIXTestLogLayout / UIXTestLogState : lines added to the test's trace, compared
//    with Refs\<test>.txt. The trace records today's behaviour (callbacks, state, layout rects)
//  - UIXTestSnapshot( "name" )                  : the rendered frame, compared with
//    Refs\<test>_<name>.png within a tolerance
//
// Run with --update-refs to write the traces and images instead of comparing them.
//------------------------------------------------------------------------------------------------

#include "UIXTestAccess.h"

typedef void (*fnUIXTest)( void );

class UIXTestRegistration
{
public:
	UIXTestRegistration( const char* szName, const char* szFile, fnUIXTest fnTest );
};

#define UIX_TEST( name )	static void name( void ); \
							static UIXTestRegistration	msReg_##name( #name, __FILE__, name ); \
							static void name( void )

#define UIX_CHECK( cond, desc )		UIXTestCheck( (cond) ? TRUE : FALSE, desc, __FILE__, __LINE__ )

//---------------------------------------------- Results
extern void		UIXTestCheck( BOOL bCondition, const char* szDescription, const char* szFile, int nLine );
extern void		UIXTestLog( const char* szFormat, ... );
extern void		UIXTestLogLayout( UIXObject* pRoot, const char* szLabel );
extern void		UIXTestSnapshot( const char* szName, int nX = 0, int nY = 0, int nW = 0, int nH = 0 );

//---------------------------------------------- Frames and input
// One frame is what an app does each frame: UIUpdate, UIX::Update, then render (UIX::Render, InterfaceDraw)
extern void		UIXTestFrame( int nFrames = 1 );
extern void		UIXTestMouseMove( int nX, int nY );
extern void		UIXTestMouseDown( void );
extern void		UIXTestMouseUp( void );
extern void		UIXTestRightDown( void );
extern void		UIXTestRightUp( void );
// Move, frame, press, frame, release, frame - as a real click arrives between frames
extern void		UIXTestClick( int nX, int nY );
extern void		UIXTestRightClick( int nX, int nY );
// Press at one point, move to another in nSteps frames, release
extern void		UIXTestDrag( int nFromX, int nFromY, int nToX, int nToY, int nSteps = 4 );
// Wheel notches as Windows reports them (positive = away from the user), passed on the way Elemental does
extern void		UIXTestWheel( int nNotches );
// Key down and up: the platform keyboard (text entry) and UIX::OnKeyUp
extern void		UIXTestKey( int nKeyCode );
// Characters into the platform keyboard, as WM_CHAR does
extern void		UIXTestType( const char* szText );

//---------------------------------------------- Helpers
extern InterfaceInstance*	UIXTestInterface( void );
extern int		UIXTestRegisterIDHandler( void );			// a button ID whose presses are logged as "press id=N param=P"
extern void		UIXTestLogPress( int nButtonID, uint32 ulParam, uint32 ulIDParam );
extern UIXRECT	UIXTestRectOf( UIXObject* pObject );		// where the object last drew (its actual render rect)
extern int		UIXTestIndexOf( UIXObject* pObject );		// a stable index for logs (raw IDs depend on test order)
extern const char*	UIXTestTypeName( UIXObject* pObject );
extern int		UIXTestIconSheet( void );

#endif
