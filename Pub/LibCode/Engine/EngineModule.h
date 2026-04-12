#ifndef ENGINE_MODULE_H
#define ENGINE_MODULE_H

// TODO - this shouldnt be here
#include "DirectX/EngineDX.h"		// Only for LPGRAPHICSDEVICE (which should be abstracted)

class EngineInstance;

class EngineModule
{
public:
	void	InitialiseModule( EngineInstance* pInstance )
	{
		mpEngineInstance = pInstance;
		mpEngineDevice = NULL;
	}
	
	void		SetGraphicsDevice( LPGRAPHICSDEVICE pDevice )
	{
		mpEngineDevice = pDevice;
	}

	LPGRAPHICSDEVICE	mpEngineDevice;
	EngineInstance*		mpEngineInstance;

};



#endif
