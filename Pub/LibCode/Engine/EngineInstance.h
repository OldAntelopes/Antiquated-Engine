#ifndef ENGINE_INSTANCE_H
#define ENGINE_INSTANCE_H

class EngineInternalsDX;


class EngineInstance
{
public:



	// TODO - Make these private and expose all the functionality through this top level interface
	EngineInternalsDX*	mpEngineInternals = NULL;


};


#endif