
#include "collmathsdefines.h"

ozbool testIntersectionLineLine(const Vec2f &_p1, const Vec2f &_p2,
								const Vec2f &_p3, const Vec2f &_p4,
								float *_t)
{
	Vec2f d1 = _p2 - _p1;
	Vec2f d2 = _p3 - _p4;

	float denom = d2.y*d1.x - d2.x*d1.y;
	if (!denom)
		return OZFALSE;

	if (_t) {
		float dist = d2.x*(_p1.y-_p3.y) - d2.y*(_p1.x-_p3.x);
		dist /= denom;
		*_t = dist;
	}

	return OZTRUE;
}

