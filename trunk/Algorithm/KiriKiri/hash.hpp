//---------------------------------------------------------
//	hashing header file
//---------------------------------------------------------
#ifndef HASH_HPP
#define HASH_HPP

#include "types.hpp"

typedef unsigned int HASH;

//---------------------------------------------------------
// sample hash creation functions
//---------------------------------------------------------
inline HASH HashRotateLeft( HASH hash, const unsigned int rotateBy )
{
	return (hash<<rotateBy)|(hash>>(32-rotateBy));
}

//---------------------------------------------------------
// create a hash from a string
//---------------------------------------------------------
template<typename T>
inline HASH MakeStringHash( const T *pString )
{
	HASH		ret = 0;
	T			c;

	if( pString )
	{
		while( c=*(pString++) )
			ret = HashRotateLeft(ret,7) + c;
	}
	return ret;
}


//---------------------------------------------------------
// create a hash from a filename
//---------------------------------------------------------
template<typename T>
inline HASH MakeFilenameHash( const T *pString )
{
	HASH			ret = 0;
	T				c;

	if( pString )
	{
		while( c=*(pString++) )
		{
			if( c>='A' && c<='Z' )
				c ^= 32;
			else if( c=='/' )
				c = '\\';
			ret = HashRotateLeft(ret,7) + c;
		}
	}
	return ret;
}



#endif
