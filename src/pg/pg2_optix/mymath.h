#ifndef MY_MATH_H_
#define MY_MATH_H_

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include "structs.h"
#include "matrix3x3.h"

template <class T> inline T min( const T a, const T b )
{
	if ( a <= b ) return a;
	return b;
}

template <class T> inline T max( const T a, const T b )
{
	if ( a >= b ) return a;
	return b;
}

template <class T> inline T sqr( const T x )
{
	return x * x;
}

inline Normal3f normalize( const Normal3f & n )
{
	float tmp = sqr( n.x ) + sqr( n.y ) + sqr( n.z );

	if ( fabsf( tmp ) > FLT_EPSILON )
	{
		tmp = 1.0f / tmp;
		return Normal3f{ n.x * tmp, n.y * tmp, n.z * tmp };
	}

	return n;
}

inline float deg2rad( const float x )
{
	return x * float( M_PI ) / 180.0f;
}

template <class T> inline float clamp( const T x, const T a, const T b )
{
	return min( max( x, a ), b );
}

inline Vector3 reflect( const Vector3 & v, const Vector3 & n )
{
	return ( 2.0f*( v.DotProduct( n ) ) )*n - v;
}

unsigned long long QuickHash( const BYTE * data, const size_t length, unsigned long long mix = 0 );

#endif
