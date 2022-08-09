
#include "collmathsdefines.h"

// This function uses the Dan Sunday's algorithm.

ozbool isPointInsideTriangle(const Vec3f &vertex0,
							 const Vec3f &vertex1,
							 const Vec3f &vertex2,
							 const Vec3f &pt)
{
	Vec3f u = vertex1 - vertex0;
	Vec3f v = vertex2 - vertex0;
	Vec3f w = pt - vertex0;

	float uu = u.dot(u);
	float uv = u.dot(v);
	float vv = v.dot(v);
	float wu = w.dot(u);
	float wv = w.dot(v);
	float d = uv * uv - uu * vv;

	float invD = 1 / d;
	float s = (uv * wv - vv * wu) * invD;
	if (s < 0 || s > 1)
		return OZFALSE;
	float t = (uv * wu - uu * wv) * invD;
	if (t < 0 || (s + t) > 1)
		return OZFALSE;

	return OZTRUE;
}

ozbool isPointInsidePolygon(int nbVertices,
							const Vec3f *pnts,
							const Vec3f &pt)
{
	int nbTriangles = nbVertices - 2;
	for (int i = 0; i < nbTriangles; i++) {
		const Vec3f &pt0 = pnts[0];
		const Vec3f &pt1 = pnts[i + 1];
		const Vec3f &pt2 = pnts[i + 2];
		if (isPointInsideTriangle(pt0, pt1, pt2, pt))
			return OZTRUE;
	}
	return OZFALSE;
}

