#ifndef UIX_PAGE_H
#define UIX_PAGE_H

#include "UIX.h"

class UIXPage : public UIXObject
{
public:
	UIXPage( uint32 uID, UIXRECT rect ) : UIXObject( uID, rect ) {}

	void		Initialise( const char* szTitle );


};





#endif