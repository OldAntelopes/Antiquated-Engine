
/***********************************************************************************
	FileName: 	sphere.h
	Author:		Igor Kravtchenko
	
	Info:		
************************************************************************************/

#ifndef OZMATH_SPHERE_H
#define OZMATH_SPHERE_H

#include "vec3f.h"

class Sphere {

public:
	Sphere() { };
	Sphere(const Vec3f &_center, float _radius) : center(_center), radius(_radius) { };

	ozinline ozbool isCollide(const Sphere &_s) const 
	{
		Vec3f diff = _s.center - center;
		float sqDist = diff.lenSq();
		float r = _s.radius + radius;
		if (sqDist <= r * r)
			return OZTRUE;
		return OZFALSE;
	}

	ozinline ozbool isPointInside(const Vec3f &_pt) const
	{
		float dist = (_pt - center).lenSq();
		return dist < (radius * radius) ? OZTRUE : OZFALSE;
	}

	Vec3f center;
	float radius;
};

#endif
