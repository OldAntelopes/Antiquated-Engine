#ifndef ANTIQUATED_PARTICLE_SYSTEM_PARTICLE_LAYER_H
#define ANTIQUATED_PARTICLE_SYSTEM_PARTICLE_LAYER_H


#include <vector>

#include "../RenderUtil/RenderObject.h"

class Particle;

class ParticleLayer : public RenderObject
{
public:
	ParticleLayer() {}

	void	Update( float delta );
	virtual int		OnPreRender();
	virtual int		OnRender();
//	int		GetGroupLayerID() { return mGroupLayerNum; }

	void		RenderAndFlush( int layerNum );

	Particle*		AddParticle( const char* szParticleTypeName, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam = 0, uint32 ulInitParamChannel = 0, void* pUserObject = NULL );


private:
	std::vector<Particle*>		mpParticleList;

	int		mGroupLayerNum;



};






#endif