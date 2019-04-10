#include "pch.h"
#include "structs.h"
#include "mymath.h"

Coord2f operator+( const Coord2f & x, const Coord2f & y )
{
	return Coord2f{ x.u + y.u, x.v + y.v };
}

Coord2f operator-( const Coord2f & x, const Coord2f & y )
{
	return Coord2f{ x.u - y.u, x.v - y.v };
}

Color3f operator+( const Color3f & x, const Color3f & y )
{
	return Color3f{ x.r + y.r, x.g + y.g, x.b + y.b };
}

Color3f operator*( const Color3f & x, const Color3f & y )
{
	return Color3f{ x.r * y.r, x.g * y.g , x.b * y.b };
}

Color3f::operator Color4f() const
{
	return Color4f{ r, g, b, 1.0f };
}

Color3f Color3f::operator*( const float x ) const
{
	return Color3f{ r * x, g * x, b * x };
}

Color3f Color3f::linear( const float gamma ) const
{	
	return Color3f{ c_linear( r ), c_linear( g ), c_linear( b ) };
}

Color3f Color3f::srgb( const float gamma ) const
{
	return Color3f{ c_srgb( r ), c_srgb( g ), c_srgb( b ) };
}

float Color3f::max_value() const
{
	return max( r, max( g, b ) );
}

bool Color3f::is_zero() const
{
	return ( ( r == 0.0f ) && ( g == 0.0f ) && ( b == 0.0f ) );
}

Vertex3f::operator Vector3() const
{
	return Vector3( x, y, z );
}

Normal3f Normal3f::operator*( const float a ) const
{
	return Normal3f{ x * a, y * a, z * a };
}

bool Color4f::is_valid() const
{
	return ( ( r == r ) && ( g == g ) && ( b == b ) && ( a == a ) );
}
