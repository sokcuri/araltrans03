//------------------------------------------------------------------------
//	types.hpp
//------------------------------------------------------------------------
//	general types
//------------------------------------------------------------------------

#ifndef TYPES_HPP
#define TYPES_HPP

typedef signed char			s8;
typedef unsigned char		u8;
typedef signed short		s16;
typedef unsigned short		u16;
typedef signed int			s32;
typedef unsigned int		u32;
typedef float				f32;
typedef double				f64;

// unicode character
typedef u16					unicode;

#define	false				((bool)0)
#define	true				((bool)1)
#define	null				(0)

//------------------------------------------------------------------------
//	increment a pointer by a given number of bytes
//------------------------------------------------------------------------
template<class X>
inline X *IncPointer( X *ptr, s32 offset )
{
    return (X*)(((u8*)ptr)+offset);
}

//------------------------------------------------------------------------
//	convert a pointer to an offset and back again
//------------------------------------------------------------------------
template<class X>
inline X *PointerToOffset( X *ptr, void *base )
{
	return (X*)(((u8*)ptr) - ((u8*)base));
}
template<class X>
inline X *OffsetToPointer( X *ptr, void *base )
{
	return (X*)IncPointer(base,*(s32*)&ptr);
}

//------------------------------------------------------------------------
//	swap
//------------------------------------------------------------------------
template<class X>
void	Swap( X &a, X &b )
{
	X	c = a;
	a = b;
	b = c;
}

//------------------------------------------------------------------------
//	min and max templates
//------------------------------------------------------------------------
template <class X>
inline X Min( const X &a, const X &b )
{
	return (a<b)?a:b;
}

template <class X>
inline X Max( const X &a, const X &b )
{
	return (a<b)?b:a;
}

//------------------------------------------------------------------------
//	align a value to the next boundary
//------------------------------------------------------------------------
inline u32 alignup( u32 value, u32 alignment )
{
	alignment = (1<<alignment)-1;
	return (value + alignment)&~alignment;
}

//------------------------------------------------------------------------
//	get the number of elements in an array
//------------------------------------------------------------------------
#ifndef countof
#define countof(a)	(sizeof(a)/sizeof(a[0]))
#endif

//------------------------------------------------------------------------
//	get the offset of an element in an object
//------------------------------------------------------------------------
#ifndef offsetof
#define offsetof(obj,member)	((u32)(&((obj*)0)->member))
#endif


#endif
