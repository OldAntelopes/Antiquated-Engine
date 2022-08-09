
/***********************************************************************************
	FileName: 	intr_sphereline.h
	Author:		Igor Kravtchenko
	
	Info:		
************************************************************************************/

#ifndef OZOSMATH_INTR_SPHERELINE_H
#define OZOSMATH_INTR_SPHERELINE_H

#include "sphere.h"

// Test the intersection between a sphere and a line
// The implementation uses standard quadratic solver and so, is not especially optimized
ozbool testIntersectionSphereLine(const Sphere &,
								  const Vec3f &pt0,
								  const Vec3f &pt1,
								  int *nbInter = NULL,
								  float *inter1 = NULL,
								  float *inter2 = NULL);

#endif
