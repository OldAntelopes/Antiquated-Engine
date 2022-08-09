
/***********************************************************************************
	FileName: 	intr_spheretri.h
	Author:		Igor Kravtchenko
	
	Info:		
************************************************************************************/

#ifndef OZMATH_INTERSECTION_SPHERETRI_H
#define OZMATH_INTERSECTION_SPHERETRI_H

#include "vec3f.h"
#include "plane.h"
#include "sphere.h"

// Test if a moving sphere intersects with a static triangle
//
// [in] triPts				an array of three pointers to point to define the triangle
// [in] triNormal			precalculated normal of the triangle
// [in] sphere				the moving sphere
// [in] sphereVel			sphere's velocity
// [out] distTravel			if a collision occurs, distance to travel before being in collision
// [out] reaction			optional, reaction vector if a collision occurs to handle some physic event
// [return]					OZTRUE if the sphere and triangle intersect, OZFALSE otherwhise
ozbool testIntersectionTriSphere(const Vec3f *triPts,
								 const Vec3f &triNormal,
								 const Sphere &sphere,
								 const Vec3f &sphereVel,
								 float &distTravel,
								 Vec3f *reaction);


#endif
