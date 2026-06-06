#ifndef MILK_COMPONENT_H
#define MILK_COMPONENT_H

class AudioInModule;

// InstanceData mirrors the classes uses in the elemental app
//  so that the milk component used there can be ported more easily to the sample
class InstanceData
{
public:

};

// ComponentBase mirrors the component classes uses in the elemental app
//  so that the milk component used there can be ported more easily to the sample
class ComponentBase
{
public:
	virtual ~ComponentBase() {}

	virtual void		OnInitialise() {}		// Note : Occurs before properties are loaded
	virtual void		OnPostInitialise() {}
	virtual void		OnUpdate( float delta, InstanceData* pCurrentInstanceData ) {}
	virtual void		OnPreRender( const InstanceData* pCurrentInstanceData ) {}
	virtual void		OnRender( const InstanceData* pCurrentInstanceData ) {}
	virtual void		OnPostFlushRender(const InstanceData* pCurrentInstanceData ) {}
	virtual void		SetDefaults() {}
	virtual void		OnDeactivateInstance() {}
	virtual void		OnActivateInstance() {}
	virtual void		OnGraphicsDeviceChanged() {}

};

//-----------------------------------------------



class MilkEmitterComponent : public ComponentBase
{
public:
	virtual ~MilkEmitterComponent();

	virtual void		OnUpdate( float delta, InstanceData* pCurrentInstanceData );
	virtual void		OnRender( const InstanceData* pCurrentInstanceData, AudioInModule* pAudio );
	virtual void		SetDefaults();
	virtual void		OnGraphicsDeviceChanged();

	void		ActivatePresetIndex( int index );

	static void		StaticShutdown();

	int		GetMilkTexture() const { return mhMilkTexture; }	

private:
	void		InitialiseMilkPlugin();

	int		mhMilkTexture = NOTFOUND;

	bool			mbHasInitialisedMilkPlugin = false;

};




#endif