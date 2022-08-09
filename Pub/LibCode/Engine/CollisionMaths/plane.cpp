
#include "collmathsdefines.h"

void Plane::fromPoints(const Vec3f &_p0, const Vec3f &_p1, const Vec3f &_p2)
{
	Vec3f v0(_p0 - _p1);
	Vec3f v1(_p2 - _p1);
	Vec3f n = v1 | v0;
	n.normalize();
	a = n.x;
	b = n.y;
	c = n.z;
	d = -(_p0.x * a + _p0.y * b + _p0.z * c);
}

void Plane::fromPointAndNormal(const Vec3f &_p, const Vec3f &_n)
{
	Vec3f nn = _n;
	nn.normalize();
	a = nn.x;
	b = nn.y;
	c = nn.z;
	d = -(_p.x * a + _p.y * b + _p.z * c);
}
