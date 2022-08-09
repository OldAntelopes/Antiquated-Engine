
/***********************************************************************************
	FileName: 	vec3f.h
	Author:		Igor Kravtchenko
	
	Info:		
************************************************************************************/

#ifndef OZOSMATH_VEC3F_H
#define OZOSMATH_VEC3F_H

#define ZEROVEC3F Vec3f(0, 0, 0)
#define UNITVEC3F Vec3f(1, 1, 1)

class Vec3f {
public:

	ozinline Vec3f()
	{
	}

	ozinline Vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{
	}

	ozinline Vec3f operator - () const
	{
		return Vec3f(-x, -y, -z);
	}

	ozinline void operator -= (const Vec3f &_v)
	{
		x -= _v.x;
		y -= _v.y;
		z -= _v.z;
	}

	ozinline void operator += (const Vec3f &_v)
	{
		x += _v.x;
		y += _v.y;
		z += _v.z;
	}

	ozinline void operator *= (float _mul)
	{
		x *= _mul;
		y *= _mul;
		z *= _mul;
	}

	ozinline void operator *= (const Vec3f &_v)
	{
		x *= _v.x;
		y *= _v.y;
		z *= _v.z;
	}

	ozinline void operator /= (float _div)
	{
		float mul = 1.0f / _div;
		x *= mul;
		y *= mul;
		z *= mul;
	}

	ozinline Vec3f operator - (const Vec3f &_v) const
	{
		return Vec3f(x - _v.x, y - _v.y, z - _v.z);
	}

	ozinline Vec3f operator + (const Vec3f &_v) const
	{
		return Vec3f(x + _v.x, y + _v.y, z + _v.z);
	}
	
	ozinline Vec3f operator * (const Vec3f &_v) const
	{
		return Vec3f(x * _v.x, y * _v.y, z * _v.z);
	}
	
	ozinline Vec3f operator * (float _m) const
	{
		return Vec3f(x * _m, y * _m, z * _m);
	}

	ozinline Vec3f operator / (const Vec3f &_v) const
	{
		return Vec3f(x / _v.x, y / _v.y, z / _v.z);
	}

	ozinline Vec3f operator / (float _d) const
	{
		float m = 1.0f / _d;
		return Vec3f(x * m, y * m, z * m);
	}

	ozinline Vec3f operator | (const Vec3f &_d) const
	{
		return Vec3f(y * _d.z - z * _d.y,
			z * _d.x - x * _d.z,
			x * _d.y - y * _d.x);
	}

	ozinline ozbool operator == (const Vec3f &_v) const
	{
		if (x == _v.x && y == _v.y && z == _v.z)
			return OZTRUE;
		return OZFALSE;
	}

	ozinline ozbool operator != (const Vec3f &_v) const
	{
		if (x != _v.x || y != _v.y || z != _v.z)
			return OZTRUE;
		return OZFALSE;
	}

	ozinline float operator [] (int _i) const
	{
		const float *val = &x;
		return val[_i];
	}

	ozinline float len() const
	{
		float len = x * x + y * y + z * z;
		return (float) sqrt(len);
	}

	ozinline float lenSq() const
	{
		return x * x + y * y + z * z;
	}

	ozinline float dot(const Vec3f &_v) const
	{
		return x * _v.x + y * _v.y + z * _v.z;
	}

	ozinline void normalize()
	{
		float ln = len();
		if (!ln)
			return;
		float div = 1.0f / ln;
		x *= div;
		y *= div;
		z *= div;
	}

	ozinline void positive()
	{
		if (x < 0) x = -x;
		if (y < 0) y = -y;
		if (z < 0) z = -z;
	}

	float x, y, z;
};

ozinline Vec3f operator * (float _m, const Vec3f &_v)
{
	return Vec3f(_v.x * _m, _v.y * _m, _v.z * _m);
}

#endif
