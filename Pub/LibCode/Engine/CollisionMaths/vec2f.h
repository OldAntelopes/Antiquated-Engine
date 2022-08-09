
/***********************************************************************************
	FileName: 	vec2f.h
	Author:		Igor Kravtchenko
	
	Info:		
************************************************************************************/

#ifndef OZMATH_VEC2F_H
#define OZMATH_VEC2F_H

class Vec2f {
public:

	ozinline Vec2f()
	{
	}

	ozinline Vec2f(float _x, float _y) : x(_x), y(_y)
	{
	}

	ozinline Vec2f operator - () const
	{
		return Vec2f(-x, -y);
	}

	ozinline void operator -= (const Vec2f &_v)
	{
		x -= _v.x;
		y -= _v.y;
	}

	ozinline void operator += (const Vec2f &_v)
	{
		x += _v.x;
		y += _v.y;
	}

	ozinline void operator *= (float _mul)
	{
		x *= _mul;
		y *= _mul;
	}

	ozinline void operator *= (const Vec2f &_v)
	{
		x *= _v.x;
		y *= _v.y;
	}

	ozinline void operator /= (float _div)
	{
		float mul = 1.0f / _div;
		x *= mul;
		y *= mul;
	}

	ozinline Vec2f operator - (const Vec2f &_v) const
	{
		return Vec2f(x - _v.x, y - _v.y);
	}

	ozinline Vec2f operator + (const Vec2f &_v) const
	{
		return Vec2f(x + _v.x, y + _v.y);
	}
	
	ozinline Vec2f operator * (const Vec2f &_v) const
	{
		return Vec2f(x * _v.x, y * _v.y);
	}
	
	ozinline Vec2f operator * (float _m) const
	{
		return Vec2f(x * _m, y * _m);
	}

	ozinline Vec2f operator / (const Vec2f &_v) const
	{
		return Vec2f(x / _v.x, y / _v.y);
	}

	ozinline Vec2f operator / (float _d) const
	{
		float m = 1.0f / _d;
		return Vec2f(x * m, y * m);
	}

	ozinline Vec2f operator | (const Vec2f &_d) const
	{
		return Vec2f(y * _d.y - x * _d.y, // FIXME!
			y * _d.x - x * _d.y);
	}

	ozinline ozbool operator == (const Vec2f &_v) const
	{
		if (x == _v.x && y == _v.y)
			return OZTRUE;
		return OZFALSE;
	}

	ozinline ozbool operator != (const Vec2f &_v) const
	{
		if (x != _v.x || y != _v.y)
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
		float len = x * x + y * y;
		return (float) sqrt(len);
	}

	ozinline float lenSq() const
	{
		return x * x + y * y;
	}

	ozinline float dot(const Vec2f &_v) const
	{
		return x * _v.x + y * _v.y;
	}

	ozinline void normalize()
	{
		float ln = len();
		if (!ln)
			return;
		float div = 1.0f / ln;
		x *= div;
		y *= div;
	}

	ozinline void positive()
	{
		if (x < 0) x = -x;
		if (y < 0) y = -y;
	}

	float x, y;
};

ozinline Vec2f operator * (float _m, const Vec2f &_v)
{
	return Vec2f(_v.x * _m, _v.y * _m);
}

#endif
