
#include "collmathsdefines.h"

ozbool testIntersectionTriSphere(const Vec3f *pTriPts,
								 const Vec3f &_triNormal,
								 const Sphere &_sphere,
								 const Vec3f &_sphereVel,
								 float &_distTravel,
								 Vec3f *_reaction)
{
	int i;
	Vec3f nvelo = _sphereVel;
	nvelo.normalize();

	if (_triNormal.dot(nvelo) > -0.001f)
		return OZFALSE;

	float minDist = FLT_MAX;
	Vec3f reaction;
	int col = -1;
	_distTravel = FLT_MAX;

	Plane plane;
	plane.fromPointAndNormal(pTriPts[0], _triNormal);

	// pass1: sphere VS plane
	float h = plane.dist( _sphere.center );
	if (h < -_sphere.radius)
		return OZFALSE;

	if (h > _sphere.radius) {
		h -= _sphere.radius;
		float dot = _triNormal.dot(nvelo);
		if (dot != 0) {
			float t = -h / dot;
			Vec3f onPlane = _sphere.center + nvelo * t;
			if (isPointInsideTriangle( pTriPts[0], pTriPts[1],pTriPts[2], onPlane)) {
				if (t < _distTravel) {
					_distTravel = t;
					if (_reaction)
						*_reaction = _triNormal;
					col = 0;
				}
			}
		}
	}

	// pass2: sphere VS triangle vertices
	for (i = 0; i < 3; i++) {
		Vec3f seg_pt0 = pTriPts[i];
		Vec3f seg_pt1 = seg_pt0 - nvelo;
		Vec3f v = seg_pt1 - seg_pt0;

		float inter1=FLT_MAX, inter2=FLT_MAX;
		int nbInter;
		ozbool res = testIntersectionSphereLine(_sphere, seg_pt0, seg_pt1, &nbInter, &inter1, &inter2);
		if (res == OZFALSE)
			continue;

		float t = inter1;
		if (inter2 < t)
			t = inter2;

		if (t < 0)
			continue;

		if (t < _distTravel) {
			_distTravel = t;
			Vec3f onSphere = seg_pt0 + v * t;
			if (_reaction)
				*_reaction = _sphere.center - onSphere;
			col = 1;
		}
	}

	// pass3: sphere VS triangle edges
	for (i = 0; i < 3; i++) {
		Vec3f edge0 = pTriPts[i];
		int j = i + 1;
		if (j == 3)
			j = 0;
		Vec3f edge1 = pTriPts[j];

		Plane plane;
		plane.fromPoints(edge0, edge1, edge1 - nvelo);
		float d = plane.dist(_sphere.center);
		if (d > _sphere.radius || d < -_sphere.radius)
			continue;

		float srr = _sphere.radius * _sphere.radius;
		float r = sqrtf(srr - d*d);

		Vec3f pt0 = plane.project(_sphere.center); // center of the sphere slice (a circle)

		Vec3f onLine;
		float h = distancePointToLine(pt0, edge0, edge1, &onLine);
		Vec3f v = onLine - pt0;
		v.normalize();
		Vec3f pt1 = v * r + pt0; // point on the sphere that will maybe collide with the edge

		int a0 = 0, a1 = 1;
		float pl_x = fabsf(plane.a);
		float pl_y = fabsf(plane.b);
		float pl_z = fabsf(plane.c);
		if (pl_x > pl_y && pl_x > pl_z) {
			a0 = 1;
			a1 = 2;
		}
		else {
			if (pl_y > pl_z) {
				a0 = 0;
				a1 = 2;
			}
		}

		Vec3f vv = pt1 + nvelo;

		float t;
		ozbool res = testIntersectionLineLine(  Vec2f(pt1[a0], pt1[a1]),
												Vec2f(vv[a0], vv[a1]),
												Vec2f(edge0[a0], edge0[a1]),
												Vec2f(edge1[a0], edge1[a1]),
												&t);
		if (!res || t < 0)
			continue;

		Vec3f inter = pt1 + nvelo * t;

		Vec3f r1 = edge0 - inter;
		Vec3f r2 = edge1 - inter;
		if (r1.dot(r2) > 0)
			continue;

		if (t > _distTravel)
			continue;

		_distTravel = t;
		if (_reaction)
			*_reaction = _sphere.center - pt1;
		col = 2;
	}

	if (_reaction && col != -1)
		_reaction->normalize();

	if ( col == -1 )
	{
		return( OZFALSE );
	}
	return( OZTRUE );
}

