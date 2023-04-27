#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "Entity.h"

extern void		EntityManagerInit( void );

extern void		EntityManagerUpdate( float delta );
extern void		EntityManagerRender( void );

extern void		EntityManagerGlobalEvent( int nEventID );

extern void		EntityManagerDeleteAllEntities( void );

extern void		EntityManagerShutdown( void );


extern Entity*		EntityManagerGetEntity( uint32 ulEntityUID );

extern Entity*		EntityManagerAddEntity( const char* szEntityTypeName, const VECT* pxPos, int nInitParam = 0 );

extern Entity*		EntityManagerGetFirstEntityOfType( const char* szEntityTypeName );

extern void			EntityManagerDeleteEntity( Entity* );

#endif