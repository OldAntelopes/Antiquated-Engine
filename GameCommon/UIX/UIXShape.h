#ifndef UIX_SHAPE_H
#define UIX_SHAPE_H

#include <string>
#include "UIX.h"

class UIXShape : public UIXObject
{
friend class UIX;
protected:
	UIXShape( uint32 uID, UIXRECT rect ) : UIXObject( uID, rect ) {}

	void	Initialise( int mode, BOOL bBlocks,uint32 ulCol1, uint32 ulCol2 );
	
	virtual UIXRECT		OnRender( InterfaceInstance* pInstance, UIXRECT pDisplayRect );

private:
	uint32				mulCol1 = 0xC0C0C0C0;
	uint32				mulCol2 = 0xC0C0C0C0;
	int					mMode = 0;
	BOOL				mBlocks = FALSE;
};






#endif