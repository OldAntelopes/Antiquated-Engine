#ifndef NATURE_TREES_H
#define NATURE_TREES_H


class Tree
{
public:

	void	Render( void );
	void	RenderTrunk( void );

	float	mfRot;
	VECT	mxPos;
	float	mfTreeSpriteSize;

	Tree*	mpNext;
};

//---------------------------------------------------------

extern void		TreesInit( void );

extern void		TreesGenerate( void );

extern void		TreesUpdate( void );

extern void		TreesRender( void );

extern void		TreesFree( void );

extern void		TreesSetView( int nGameMapMinX, int nGameMapMinY, int nGameMapMaxX, int nGameMapMaxY );

extern Tree*	TreesGetRandomTree( int nGameMapMinX, int nGameMapMinY, int nGameMapMaxX, int nGameMapMaxY );


#endif