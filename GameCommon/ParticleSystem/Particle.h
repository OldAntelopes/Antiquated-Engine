#ifndef PARTICLE_H
#define PARTICLE_H

#define		IN_MORGUE	-1

#include "../RenderUtil/Sprites3D.h"

class Particle
{
public:
	Particle();
	virtual ~Particle();

	virtual void		OnInit( int nInitParam ) {}
	virtual void		OnUpdate( float delta ) {}
	virtual void		OnRender( void ) {}

	virtual BOOL		UseDefaultRender( void ) { return( TRUE ); }

	void	Init( int typeID, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParm = 0 );
	void	Update( float fDelta );
	virtual void	Render( void );
	
	void	SetGraphic( const char* szSpriteTextureName, float fGridScale, BOOL bUseRotation = FALSE, eSpriteGroupRenderFlags renderFlags = kSpriteRender_Default );

	const VECT*	GetPos( void ) { return( &mxPos ); }
	const VECT*	GetVel( void ) { return( &mxVel ); }
	float		GetRot( void ) { return( mfRot ); }

	void	SetPos( const VECT* pxPos ) { mxPos = *pxPos; }
	void	SetVel( const VECT* pxVel ) { mxVel = *pxVel; }
	void	SetRot( float fRot ) { mfRot = fRot; }
	void	SetCol( uint32 ulCol ) { mulCol = ulCol; }

	void	SetSpriteScale( float fScale ) { mfSpriteScale = fScale; }
	void	SetSpriteFrameNum( int nFrameNum ) { mnSpriteFrameNum = nFrameNum; }

	int		GetParticleGraphicNum( void ) { return( mnParticleGraphicsNum ); }

	int		GetTypeID( void ) { return( mType ); }
	void	SetTypeID( int type ) { mType = type; }
	
	float	GetLongevity( void ) { return( mfLongevity ); }
	float	GetTimeAlive( void ) { return( mfTimeAlive ); }
	uint32	GetCol( void ) { return( mulCol ); }

	void		SetNext( Particle* pNext ) { mpNext = pNext; }
	Particle*	GetNext( void ) { return( mpNext ); }

private:
	void		DefaultRender( void );
	
	int			mnParticleGraphicsNum;
	VECT		mxPos;
	VECT		mxVel;
	int			mType;
	uint32		mulCol;
	float		mfLongevity;
	float		mfTimeAlive;
	float		mfSpriteScale;
	float		mfRot;
	int			mnSpriteFrameNum;

	Particle*	mpNext;

};


class AnimatedParticle : public Particle
{
public:
	AnimatedParticle();

	virtual void	Render( void );

private:
	int			mnAnimFrameStart;
	int			mnAnimFrameEnd;
	float		mfAnimPhase;
	float		mfAnimSpeed;

};

//---------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------
// Particle Registration
// 
// All Particle cpp should include the define 
//  
// REGISTER_Particle( [class_name], [text_name] )
//
//--------------------------------------------------------------------

typedef	Particle*	(*ParticleNewFunction)( void );

class RegisteredParticleList
{
public:
	static void		Shutdown( void );
	static BOOL		Register( const char* szParticleName, ParticleNewFunction fnNewParticle );

	char*					mszParticleName;
	ParticleNewFunction		mfnParticleNew;
	Particle*					mspActiveParticleList;
	int						mnParticleTypeID;

	RegisteredParticleList*		mpNext;
	
};


// this registers a derived class in the factory method of the base class
// it adds a factory function named create_NAME()
// and calls Base::reg() by the help of a dummy static variable to register the function
#define REGISTER_PARTICLE(_classname,_textname) \
namespace { \
	Particle* create_ ## _classname() {  return new _classname; } \
	static BOOL _classname ## _creator_registered = RegisteredParticleList::Register( _textname, create_ ## _classname); }





#endif
