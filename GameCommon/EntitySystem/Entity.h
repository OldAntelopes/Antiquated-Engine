#ifndef ENTITY_H
#define ENTITY_H

#include "Components\Component.h"

#define		IN_MORGUE	-1

class Entity
{
public:
	Entity();
	virtual ~Entity();

	virtual void		OnInit( int nInitParam ) {}
	virtual void		OnAddToWorld( void ) {}
	virtual void		OnUpdate( float delta ) {}
	virtual void		OnRender( void ) {}
	virtual void		OnEntityAnimationUpdate( int hModelNum ) {}
	virtual BOOL		UseDefaultRender( void ) { return( TRUE ); }

	void	Init( ulong ulEntityUID, int typeID, const VECT* pxPos, int nInitParam = 0 );
	void	AddToWorld( void );
	void	Update( float fDelta );
	void	Render( void );
	
	void	SetGraphic( const char* szModelName, const char* szTextureName, int nInstanceNum );

	ulong	GetEntityUID( void ) { return( mulEntityUID ); }

	const VECT*	GetPos( void ) { return( &mxPos ); }
	const ENGINEQUATERNION*	GetOrientation( void ) { return( &mxOrientation ); }
	const VECT*	GetRot( void ) { return( &mxRot ); }
	const VECT*	GetVel( void ) { return( &mxVel ); }

	void	GetForward( VECT* pxOut );
	void	GetUp( VECT* pxOut );
	void	GetRight( VECT* pxOut );

	void	SetRot( const VECT* pxRot ) { mxRot = *pxRot; }
	void	SetPos( const VECT* pxPos ) { mxPos = *pxPos; }
	void	SetVel( const VECT* pxVel );

	void	Respawn( const VECT* pxPos );
	void	SetOrientation( const ENGINEQUATERNION* pQuat ) { mxOrientation = *pQuat; mbUsingQuaternion = TRUE; }

	int		GetEntityGraphicNum( void ) { return( mnEntityGraphicsNum ); }

	BOOL	SetDestination( VECT* pxDestination );
	BOOL	SetImmediateDestination( VECT* pxDestination );
	BOOL	HasDestination( void );
	BOOL	ArrivedAtImmediateDestination( void );

	void	Event( int nEventID );

	Component*	AddComponent( const char* szComponentType );
	Component*	GetComponent( eCompronentBehaviourTypes behaviourType );
	Component*	GetNamedComponent( const char* szComponentType );

	int		GetTypeID( void ) { return( mType ); }
	void	SetTypeID( int type ) { mType = type; }

	void	SetNext( Entity* pNext ) { mpNext = pNext; }
	Entity*	GetNext( void ) { return( mpNext ); }

	void	SetVelInternal( const VECT* pxVel ) { mxVel = *pxVel; }
private:
	void		DefaultRender( void );
	void		UpdateTargetTravel( float fDelta );
	
	ulong		mulEntityUID;
	int			mnEntityGraphicsNum;
	VECT		mxPos;
	VECT		mxRot;
	VECT		mxVel;

	ENGINEQUATERNION	mxOrientation;
	BOOL		mbUsingQuaternion;

	int			mType;
	Component*		mpComponentList;
	Entity*			mpNext;

};



//---------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------
// Entity Registration
// 
// All Entity cpp should include the define 
//  
// REGISTER_ENTITY( [class_name], [text_name] )
//
//--------------------------------------------------------------------

typedef	Entity*	(*EntityNewFunction)( void );

class RegisteredEntityList
{
public:
	static void		Shutdown( void );
	static BOOL		Register( const char* szEntityName, EntityNewFunction fnNewEntity );

	char*					mszEntityName;
	EntityNewFunction		mfnEntityNew;
	Entity*					mspActiveEntityList;
	int						mnEntityTypeID;

	RegisteredEntityList*		mpNext;
	
};


// this registers a derived class in the factory method of the base class
// it adds a factory function named create_NAME()
// and calls Base::reg() by the help of a dummy static variable to register the function
// e.g. REGISTER_ENTITY( MyEntityClass, "MyEntity" )
#define REGISTER_ENTITY(_classname,_textname) \
namespace { \
	Entity* create_ ## _classname() {  return new _classname; } \
	static BOOL _classname ## _creator_registered = RegisteredEntityList::Register( _textname, create_ ## _classname); }





#endif
