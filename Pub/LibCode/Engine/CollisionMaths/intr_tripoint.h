
/***********************************************************************************
	FileName: 	intr_tripoint.h
	Author:		Igor Kravtchenko
	
	Info:		
************************************************************************************/

#ifndef OZMATH_INTR_TRIPOINT_H
#define OZMATH_INTR_TRIPOINT_H

#include "vec3f.h"

// Test if a point is inside a triangle (in a pretty speedy way) given three points.

ozbool isPointInsideTriangle(const Vec3f &vertex0,
							 const Vec3f &vertex1,
							 const Vec3f &vertex2,
							 const Vec3f &pt);

ozbool isPointInsidePolygon(int nbVertices,
						    const Vec3f *pnts,
							const Vec3f &pt);

#endif
