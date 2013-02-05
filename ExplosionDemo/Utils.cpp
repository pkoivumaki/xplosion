#include "Utils.h"



float VecMath::Length( const Vec& v )
{
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

Vec VecMath::Normalize(const Vec& v)
{
	float l = Length(v);
	return Vec(v.x/l, v.y/l, v.z/l);
}

float VecMath::Dot(const Vec& lhs, const Vec& rhs)
{
	return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

Vec VecMath::Cross(const Vec& lhs, const Vec& rhs)
{
	return Vec(lhs.y*rhs.z - lhs.z*rhs.y, lhs.z*rhs.x - lhs.x*rhs.z, lhs.x*rhs.y - lhs.y*rhs.x);
}

/*
float Vec::Length() const
{
	return sqrt(x*x+y*y+z*z);
}

Vec Vec::Normalize() const
{
	float l = Length();
	return Vec(x/l, y/l, z/l);
}

float Vec::Dot( const Vec& other ) const
{
	return x*other.x + y*other.y + z*other.z;
}

Vec Vec::Cross( const Vec& other ) const
{
	return Vec(y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x);
}

Vec Vec::operator-() const
{
	return Vec(x, y, z);
}

Vec& Vec::operator+=( const Vec& other )
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

Vec& Vec::operator-=( const Vec& other )
{
	*this+= -other;
	return *this;
}

Vec& Vec::operator*=( float a )
{
	x *= a;
	y *= a;
	z *= a;
	return *this;
}

Vec& Vec::operator/=( float a )
{
	x /= a;
	y /= a;
	z /= a;
	return *this;
}

Vec Vec::operator+( const Vec& other ) const
{
	return Vec(x+other.x, y+other.y, z+other.z);
}

Vec Vec::operator-( const Vec& other ) const
{
	return Vec(x-other.x, y-other.y, z-other.z);
}

Vec Vec::operator*( float a ) const
{
	return Vec(x*a, y*a, z*a);
}

Vec Vec::operator/( float a ) const
{
	return Vec(x/a, y/a, z/a);
}



*/
