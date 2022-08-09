#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	float		fIndex;
	float		fValue;

} GRAPH_STRUCT;


extern float	GraphStructGetValue( GRAPH_STRUCT* pxGraphStruct, float fIndex );

extern float	RotateAngleTowardsAngle( float fAngleToChange, float fTargetAngle, float fTurnRate );

extern float	GetDestinationRotZ( const VECT* pxDestination, const VECT* pxPos );
extern float	GetDestinationPitch( const VECT* pxDestination, const VECT* pxPos );

extern float	WrapAngle360( float fAngle );


#ifdef __cplusplus
}
#endif



#endif
