#ifndef GENERATOR_H
#define GENERATOR_H


typedef struct
{
	BOOL	bGenShadows;
	BOOL	bGenHiRes;
	BOOL	bGenPathfinding;
	BOOL	bGenLoRes;
	BOOL	bGenTreeShadows;

} GENERATOR_CONFIG;


extern void		GeneratorInitPreset( GENERATOR_CONFIG* pConfig );
extern BOOL		GeneratorUpdate( void );

extern void		GeneratorDisplay( int X, int Y, int W, int H );

extern BOOL		GeneratorIsActive( void );

extern void		GeneratorShutdown( void );


#endif