#ifndef COLLISION_MATHS_DEFINES_H
#define COLLISION_MATHS_DEFINES_H

#ifdef __GNUC__
#define ozinline __inline__
#else
#define ozinline _inline
#endif


#define ozbool bool

#define OZTRUE true
#define OZFALSE false

#include <float.h>
#include <math.h>
#include <stdio.h>

#include "dist_pointline.h"
#include "intr_lineline.h"
#include "intr_sphereline.h"
#include "intr_spheretri.h"
#include "intr_tripoint.h"
#include "plane.h"
#include "sphere.h"
#include "vec2f.h"
#include "vec3f.h"


#endif // COLLISION_MATHS_DEFINES_H