
#include "collmathsdefines.h"

#define square(a) ((a)*(a))

ozbool testIntersectionSphereLine(const Sphere &_sphere,
								  const Vec3f &_pt0,
								  const Vec3f &_pt1,
								  int *_nbInter,
								  float *_inter1,
								  float *_inter2)
{
	float a, b, c, i;

	a = square(_pt1.x - _pt0.x) + square(_pt1.y - _pt0.y) + square(_pt1.z - _pt0.z);
	b =  2 * ( (_pt1.x - _pt0.x) * (_pt0.x - _sphere.center.x)
		+ (_pt1.y - _pt0.y) * (_pt0.y - _sphere.center.y)
		+ (_pt1.z - _pt0.z) * (_pt0.z - _sphere.center.z) ) ;
	c = square(_sphere.center.x) + square(_sphere.center.y) +
		square(_sphere.center.z) + square(_pt0.x) +
		square(_pt0.y) + square(_pt0.z) -
		2 * ( _sphere.center.x * _pt0.x + _sphere.center.y * _pt0.y + _sphere.center.z * _pt0.z ) - square(_sphere.radius) ;
	i =  b * b - 4 * a * c;

	if (i < 0)
		return OZFALSE;

	if (i == 0) {
		if (_nbInter) *_nbInter = 1;
		if (_inter1) *_inter1 = -b / (2 * a);
	}
	else {
		if (_nbInter) *_nbInter = 2;
		if (_inter1) *_inter1 = (-b + sqrtf( square(b) - 4*a*c )) / (2 * a);
		if (_inter2) *_inter2 = (-b - sqrtf( square(b) - 4*a*c )) / (2 * a);
	}

	return OZTRUE;
}

