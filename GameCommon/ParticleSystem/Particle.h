#ifndef PARTICLE_H
#define PARTICLE_H

#define		IN_MORGUE	0xFFFF


#include <map>

#include "../RenderUtil/Sprites3D.h"

class MultiVertexBuffers;

class Particle
{
public:
	Particle();
	virtual ~Particle();

	virtual void		OnInit( int nInitParam, void* pUserObject ) {}
	virtual void		OnInitComplete( int nInitParam, void* pUserObject ) {}
	virtual void		OnUpdate( float delta ) {}
	virtual void		OnRenderParticle( MultiVertexBuffers* pVertexBuff, uint32 ulRenderFlags ) {}
	virtual void		OnPreRenderParticle() {}

	virtual BOOL		UseDefaultRender( void ) { return( TRUE ); }
	virtual float		GetAlphaOverride( void ) { return -1.0f; }

	void	Init( int typeID, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParm = 0, ushort uwInitParamChannel = 0, void* pUserObject = NULL, ushort uwSpriteRenderLayer = 0 );
	void	Update( float fDelta );
	virtual void	RenderParticle( MultiVertexBuffers* pVertexBuff, uint32 ulRenderFlags );
	void	PreRender();

	void	SetGraphic( const char* szSpriteTextureName, float fGridScale, BOOL bUseRotation = FALSE, eRenderFlags renderFlags = kRenderFlag_Default, int layer = 0 );
	void	SetGraphicHandle( int hTex, float fGridScale, BOOL bUseRotation = FALSE, eRenderFlags renderFlags = kRenderFlag_Default, int layer = 0 );

	const VECT*	GetPos( void ) const { return( &mxPos ); }
	const VECT*	GetVel( void ) const { return( &mxVel ); }
	const VECT*	GetFacingDirection( void ) const { return( &mxDir ); }
	const VECT*	GetFacingUp( void ) const { return( &mxUp ); }
	const VECT*	GetOffset( void ) const { return( &mxOffset ); }
	const VECT*	GetFieldVect( void ) const { return( &mxFieldVect ); }
	float		GetRot( void ) const { return( mfRot ); }
	float		GetRotDeg( void ) const { return( RADTODEG(mfRot) ); }
	float		GetRotSpeed( void ) const { return( mfRotSpeed ); }

	void	SetPos( const VECT* pxPos ) { mxPos = *pxPos; }
	void	SetVel( const VECT* pxVel ) { mxVel = *pxVel; }
	void	SetFacingDirection( const VECT* pxDir ) { mxDir = *pxDir; }
	void	SetFieldVect( const VECT* pxFieldVect ) { mxFieldVect = *pxFieldVect; }
	void	SetRot( float fRot ) { mfRot = fRot; }
	void	SetRotDeg( float fRotDeg ) { mfRot = DEGTORAD(fRotDeg); }
	void	SetRotSpeed( float fRotSpeed ) { mfRotSpeed = fRotSpeed; }
	void	SetOffset( const VECT* pxOffset ) { mxOffset = *pxOffset; }
	void	SetCol( uint32 ulCol ) { mulCol = ulCol; }

	void	SetFadeInTime( float fTimeSecs ) { mfFadeInTime = fTimeSecs; }
	void	SetSpriteScale( float fScale ) { mfSpriteScale = fScale; }
	void	SetSpriteAspect( float fAspectRatio ) { mfSpriteAspect = fAspectRatio; }
	void	SetSpriteFrameNum( ushort uwFrameNum ) { muwSpriteFrameNum = uwFrameNum; }
	void	SetParamChannel( ushort uwChannel ) { muwParamChannel = uwChannel; }

	int		GetParticleGraphicNum( void ) const { return( mnParticleGraphicsNum ); }

	void	Finalise();		// Called at end of init process to store base values for things like scale and colour which can be modified by components

	int		GetTypeID( void ) const { return( mType ); }
	void	SetTypeID( ushort type ) { mType = type; }
	
	float	GetLongevity( void ) const { return( mfLongevity ); }
	float	GetTimeAlive( void ) const { return( mfTimeAlive ); }
	float	GetSpriteScale( void ) const { return( mfSpriteScale ); }
	float	GetSpriteAspect( void ) const { return( mfSpriteAspect ); }
	uint32	GetCol( void ) const { return( mulCol ); }
	ushort	GetParamChannel( void ) const { return( muwParamChannel ); }
	ushort	GetSpriteRenderLayer() const { return(muwSpriteRenderLayer); }

	float	GetBaseScale( void ) const { return( mfBaseScale ); }
	uint32	GetBaseCol(void) const { return(mulBaseCol); }	

	void		SetNext( Particle* pNext ) { mpNext = pNext; }
	Particle*	GetNext( void ) const { return( mpNext ); }
	void		KillSelf() { mType = IN_MORGUE; }
protected:
	void		AddVertices( MultiVertexBuffers* pVertexBuff, uint32 ulRenderFlags, uint32 ulCol );

	int			mnParticleGraphicsNum = NOTFOUND;
	float		mfTimeAlive = 0.0f;
	float		mfLongevity;
	float		mfFadeInTime = 0.0f;
	float		mfSpriteScale;
	float		mfSpriteAspect = 1.0f;
	VECT		mxPos;
	VECT		mxVel;
	VECT		mxDir;
	VECT		mxUp = VECT(0.0f,-1.0f,0.0f);		// By default, particles are aligned so they face up in the negative y (they travel around in the XZ plane)
	VECT		mxOffset;
	VECT		mxFieldVect;
	uint32		mulCol;
	float		mfRot = 0.0f;
	float		mfRotSpeed = 0.0f;
	ushort		mType;
	ushort		muwSpriteFrameNum;		
	ushort		muwParamChannel;			
	ushort		muwSpriteRenderLayer = 0;	

	float		mfBaseScale = 1.0f;
	uint32		mulBaseCol = 0;
private:
	void		DefaultRender( MultiVertexBuffers* pVertexBuff, uint32 ulRenderFlags );
	void		SetBaseScale( float fScale ) { mfBaseScale = fScale; }
	void		SetBaseCol(uint32 ulCol) { mulBaseCol = ulCol; }	

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
	std::map<int,Particle*>		mParticleLayerMap;
//	Particle*				mspActiveParticleList;
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
