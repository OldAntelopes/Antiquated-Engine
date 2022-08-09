
/***********************************************************************************
	FileName: 	intr_lineline.h
	Author:		Igor Kravtchenko
	
	Info:		
************************************************************************************/

#ifndef OZOSMATH_INTR_LINELINE_H
#define OZOSMATH_INTR_LINELINE_H

#include "vec2f.h"

// test if two 2D lines (p0, p1) and (p2, p3) intersect
// if they do OZTRUE is returned, OZFALSE otherwhise
ozbool testIntersectionLineLine(const Vec2f &p1, const Vec2f &p2,
								const Vec2f &p3, const Vec2f &p4,
								float *t);

#endif
