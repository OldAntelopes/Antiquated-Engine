
/***********************************************************************************
	FileName: 	dist_pointline.h
	Author:		Igor Kravtchenko
	
	Info:		
************************************************************************************/

#ifndef OZOSMATH_DIST_POINTLINE_H
#define OZOSMATH_DIST_POINTLINE_H

#include "vec3f.h"

float sqrDistancePointToLine(const Vec3f &point,
						     const Vec3f &pt0,
							 const Vec3f &pt1,
							 Vec3f *linePt = NULL);

float distancePointToLine(const Vec3f &point,
						  const Vec3f &pt0,
						  const Vec3f &pt1,
						  Vec3f *linePt = NULL);

#endif
