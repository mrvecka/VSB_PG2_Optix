#include "pch.h"
#include "mymath.h"

unsigned long long QuickHash( const BYTE * data, const size_t length, unsigned long long mix )
{
	// set 'mix' to some value other than zero if you want a tagged hash          
	const unsigned long long mulp = 2654435789;
	mix ^= 104395301;

	for ( size_t i = 0; i < length; ++i )
	{
		mix += ( data[i] * mulp ) ^ ( mix >> 23 );
	}

	return mix ^ ( mix << 37 );
}
