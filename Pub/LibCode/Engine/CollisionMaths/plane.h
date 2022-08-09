
/***********************************************************************************
	FileName: 	plane.h
	Author:		Igor Kravtchenko
	
	Info:		
************************************************************************************/

#ifndef OZMATH_PLANE_H
#define OZMATH_PLANE_H

#include "vec3f.h"

enum PLANE {
	LEFT_PLANE, RIGHT_PLANE,
	BOTTOM_PLANE, TOP_PLANE,
	NEAR_PLANE, FAR_PLANE
};

class Plane 
{
public:

	Plane() { a = 0.0f; b = 0.0f; c = 0.0f; d = 1.0f; };
	Plane(float _a, float _b, float _c, float _d) : a(_a), b(_b), c(_c), d(_d) { };

	void fromPoints(const Vec3f &p_0, const Vec3f &_p1, const Vec3f &_p2);
	void fromPointAndNormal(const Vec3f &_p, const Vec3f &_n);

	ozinline float dot(const Vec3f &_p) const
	{
		return a * _p.x + b * _p.y + c * _p.z;
	}

	ozinline float dist(const Vec3f &_p) const
	{
		return a * _p.x + b * _p.y + c * _p.z + d;
	}

	ozinline Vec3f reflect(const Vec3f &_vec)
	{
		float d = dist(_vec);
		return _vec + 2 * Vec3f(-a, -b, -c) * d;
	}

	ozinline Vec3f project(const Vec3f &_p)
	{
		float h = dist(_p);
		return Vec3f(_p.x - a * h,
					 _p.y - b * h,
					 _p.z - c * h);
	}

	ozinline ozbool isOnPlane(const Vec3f &_p, float _threshold = 0.001f)
	{
		float d = dist(_p);
		if (d < _threshold && d > -_threshold)
			return OZTRUE;
		return OZFALSE;
	}

	// Calcul the intersection between this plane and a line
	// If the plane and the line are parallel, OZFALSE is returned
	ozinline ozbool intersectWithLine(const Vec3f &_p0, const Vec3f &_p1, float &_t)
	{
		Vec3f dir = _p1 - _p0;
		float div = dot(dir);
		if (div == 0)
			return OZFALSE;

		_t = -dist(_p0) / div;
		return OZTRUE;
	}

	float a, b, c, d;
};

#endif
