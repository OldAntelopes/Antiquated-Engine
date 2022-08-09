#ifndef ENTITY_COMPONENTS_H
#define ENTITY_COMPONENTS_H

enum eComponentTypes
{
	PATHFINDING_COMPONENT,
	GROUND_MOTION_COMPONENT,
	VEHICLE_CONTROLLER_COMPONENT,
	SIMPLE_TRAVEL_COMPONENT,
};

enum eCompronentBehaviourTypes
{
	CONTROL_BEHAVIOUR_COMPONENT,
	MOVEMENT_BEHAVIOUR_COMPONENT,
};

class Entity;

class Component
{
public:
	Component()
	{
		mpNext = NULL;
		mpParentEntity = NULL;
	}

	virtual ~Component()
	{

	}

	static Component*		Create( const char* szComponentType );

	void	AddComponentToEntity( Entity* pEntity );

	void	UpdateComponent( float fDelta );

	void	RenderComponent( void );

	void	ReleaseComponent( void );
		
	virtual eCompronentBehaviourTypes	GetBehaviourType( void ) = 0;

	virtual void		OnUpdateComponent( float fDelta ) {}
	virtual void		OnRenderComponent( void ) {}
	virtual void		OnEntityInitialised( void ) {}
	virtual void		OnEntityAddToWorld( void ) {}
	virtual void		OnEvent( int nEventID ) {}

	virtual BOOL		OnEntitySetDestination( const VECT* pxDestination ) { return( FALSE ); }
	virtual BOOL		OnEntitySetImmediateDestination( const VECT* pxDestination ) { return( FALSE ); }
	virtual BOOL		OnEntityArrivedAtImmediateDestination( void ) { return( FALSE ); }
	virtual	BOOL		MoveToDestination( float fDelta, const VECT* pxDestination ) { return( FALSE ); }
	virtual BOOL		EntityAnimationUpdate( int hModel ) { return( FALSE ); }
	virtual void		OnEntitySetVel( const VECT* pxVel ) {}
	virtual void		OnEntityRespawn( const VECT* pxPos ) {}

	virtual const char*			GetComponentType( void ) = 0;

	Entity*				GetEntity( void ) { return( mpParentEntity ); }

	Component*		GetNext( void ) { return( mpNext ); }
	void			SetNext( Component* pNext ) { mpNext = pNext; }
private:
	
	Entity*				mpParentEntity;
	Component*			mpNext;

};

class MovementComponent : public Component
{
public:
	virtual BOOL		HasDestination( void ) = 0;
	
};


//---------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------
// Component Registration
// 
// All component.cpp should include the define 
//  
// REGISTER_COMPONENT( [name], [newFunction] )
//
// with the newFunction returning a new instance of the component class
//--------------------------------------------------------------------

typedef	Component*	(*ComponentNewFunction)( void );

class RegisteredComponentList
{
public:
	static void		Shutdown( void );
	static BOOL		Register( const char* szComponentName, ComponentNewFunction fnNewComponent );

	char*					mszComponentName;
	ComponentNewFunction	mfnComponentNew;

	RegisteredComponentList*		mpNext;
	
};


// this registers a derived class in the factory method of the base class
// it adds a factory function named create_NAME()
// and calls Base::reg() by the help of a dummy static variable to register the function
#define REGISTER_COMPONENT(_classname,_textname) \
namespace { \
	Component* create_ ## _classname() {  return new _classname; } \
	static BOOL _classname ## _creator_registered = RegisteredComponentList::Register( _textname, create_ ## _classname); }




#endif