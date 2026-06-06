#ifndef MILK_PLAYER_H
#define MILK_PLAYER_H


class MilkEmitterComponent;
class AudioInModule;

class MilkPlayerSample
{
public:
	static MilkPlayerSample& GetSingleton() 
	{
		static MilkPlayerSample msSingleton; 
		return msSingleton; 
	}

	void	Initialise();	
	void	Update( float delta );
	void	Render();
	void	Shutdown();
private:

	MilkEmitterComponent*		mpMilkComponent = NULL;
	AudioInModule*				mpAudioInModule = NULL;
};






#endif