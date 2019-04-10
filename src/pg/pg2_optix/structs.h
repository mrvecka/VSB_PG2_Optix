#pragma once

#include "vector3.h"

/* a single vertex position structure matching Embree format */
struct Vertex3f
{
	float x, y, z;

	operator Vector3() const;
};

struct Normal3f : public Vertex3f
{
	/*float dot( const Vector3 & v ) const
	{
		return x * v.x + y * v.y + z * v.z;
	}*/
	Normal3f( const float x, const float y, const float z )
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	/* reorient the normal vector againt the incoming ray with direction (v_x, v_y, v_z) */
	void unify( const float v_x, const float v_y, const float v_z )
	{
		if ( ( x * v_x + y * v_y + z * v_z ) > 0.0f )
		{
			x *= -1.0f;
			y *= -1.0f;
			z *= -1.0f;
		}

		// !!! always renormalize the normal otherwise bright artifacts at face edges will appear !!!
		Vector3 n( x, y, z );
		n.Normalize();
		x = n.x;
		y = n.y;
		z = n.z;
	}

	Normal3f operator* ( const float a ) const;
};

struct Coord2f { float u, v; }; // texture coord structure

Coord2f operator+ ( const Coord2f & x, const Coord2f & y );
Coord2f operator- ( const Coord2f & x, const Coord2f & y );

struct Triangle3ui { unsigned int v0, v1, v2; }; // indicies of a single triangle, the struct must match certain format, e.g. RTC_FORMAT_UINT3

inline float c_linear( const float c_srgb, const float gamma = 2.4f )
{
	if ( c_srgb <= 0.0f ) return 0.0f;
	else if ( c_srgb >= 1.0f ) return 1.0f;
	
	assert( ( c_srgb >= 0.0f ) && ( c_srgb <= 1.0f ) );

	if ( c_srgb <= 0.04045f )
	{
		return c_srgb / 12.92f;
	}
	else
	{
		const float a = 0.055f;
		return powf( ( c_srgb + a ) / ( 1.0f + a ), gamma );
	}
}

inline float c_srgb( const float c_linear, const float gamma = 2.4f )
{
	if ( c_linear <= 0.0f ) return 0.0f;
	else if ( c_linear >= 1.0f ) return 1.0f;

	assert( ( c_linear >= 0.0f ) && ( c_linear <= 1.0f ) );

	if ( c_linear <= 0.0031308f )
	{
		return 12.92f * c_linear;
	}
	else
	{
		const float a = 0.055f;
		return ( 1.0f + a ) * powf( c_linear, 1.0f / gamma ) - a;
	}
}

struct Color4f;

struct Color3f
{
	float r, g, b;

	Color3f( const float r = 0.0f, const float g = 0.0f, const float b = 0.0f ) : r( r ), g( g ), b( b ) { }

	operator Color4f() const;

	Color3f operator* ( const float x ) const;

	Color3f linear( const float gamma = 2.4f ) const;
	Color3f srgb( const float gamma = 2.4f ) const;

	float max_value() const;

	template<typename T> static Color3f make_from_bgr( BYTE * p )
	{
		return Color3f{ float( ( ( T* )( p ) )[2] ), float( ( ( T* )( p ) )[1] ), float( ( ( T* )( p ) )[0] ) };
	}

	bool is_zero() const;
};

Color3f operator+ ( const Color3f & x, const Color3f & y );
Color3f operator* ( const Color3f & x, const Color3f & y );

struct Color4f /*: public Color3f*/
{
	float r, g, b;
	float a; // a = 1.0 means that the pixel is fully opaque		

	bool is_valid() const;
};
