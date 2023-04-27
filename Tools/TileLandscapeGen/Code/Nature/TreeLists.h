
#ifndef TREE_LISTS_H
#define TREE_LISTS_H

class Tree;

typedef	void(*fnProcessTreeCallback)( Tree*, uint32 );


void	TreeListsInit( void );


void	TreeListsAddTree( Tree* );

void	TreeListsUpdate( float fDelta );

void	TreeListsSetView( int nGameMapMinX, int nGameMapMinY, int nGameMapMaxX, int nGameMapMaxY );

void	TreeListsRenderPass1( void );
void	TreeListsRenderPass2( void );

void	TreeListsProcessView( fnProcessTreeCallback, uint32 ulParam  );

void	TreeListsFree( void );



#endif
