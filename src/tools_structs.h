#ifndef TOOLS_STRUCTS_H
#define TOOLS_STRUCTS_H

#ifdef __BIG_ENDIAN__Q__
#define BigShort(x)		(x)
#define BigLong(x)		(x)
#define BigFloat(x)		(x)
#define LittleShort(x)	ShortSwap(x)
#define LittleLong(x)	LongSwap(x)
#define LittleFloat(x)	FloatSwap(x)
#elif defined(__LITTLE_ENDIAN__Q__)
#define BigShort(x)		ShortSwap(x)
#define BigLong(x)		LongSwap(x)
#define BigFloat(x)		FloatSwap(x)
#define LittleShort(x)	(x)
#define LittleLong(x)	(x)
#define LittleFloat(x)	(x)
#elif defined(__PDP_ENDIAN__Q__)
int		LongSwapPDP2Big (int l);
int		LongSwapPDP2Lit (int l);
float	FloatSwapPDP2Big (float f);
float	FloatSwapPDP2Lit (float f);
#define BigShort(x)		ShortSwap(x)
#define BigLong(x)		LongSwapPDP2Big(x)
#define BigFloat(x)		FloatSwapPDP2Big(x)
#define LittleShort(x)	(x)
#define LittleLong(x)	LongSwapPDP2Lit(x)
#define LittleFloat(x)	FloatSwapPDP2Lit(x)
#else
#error Unknown byte order type!
#endif
#endif


