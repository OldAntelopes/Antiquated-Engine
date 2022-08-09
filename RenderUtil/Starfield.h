
#ifndef	GAMECOMMON_STARFIELD_H
#define	GAMECOMMON_STARFIELD_H

enum eStarfieldType
{
	STARFIELDTYPE_3DVIEW,
	STARFIELDTYPE_2DVIEW,
};

extern void		StarfieldInit( eStarfieldType );

extern void		StarfieldRender( void );

extern void		StarfieldShutdown( void );







#endif

