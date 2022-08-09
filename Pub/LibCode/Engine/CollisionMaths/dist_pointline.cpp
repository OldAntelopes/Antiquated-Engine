
#include "collmathsdefines.h"

float sqrDistancePointToLine(const Vec3f &_point,
							 const Vec3f &_pt0,
							 const Vec3f &_pt1,
							 Vec3f *_linePt)
{
	Vec3f v = _point - _pt0;
	Vec3f s = _pt1 - _pt0;
	float lenSq = s.lenSq();
	float dot = v.dot(s) / lenSq;
	Vec3f disp = s * dot;
	if (_linePt)
		*_linePt = _pt0 + disp;
	v -= disp;
	return v.lenSq();
}

float distancePointToLine(const Vec3f &_point,
						  const Vec3f &_pt0,
						  const Vec3f &_pt1,
						  Vec3f *_linePt)
{ 
	return sqrtf( sqrDistancePointToLine(_point, _pt0, _pt1, _linePt) );
}

