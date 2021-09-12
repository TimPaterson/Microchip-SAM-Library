//****************************************************************************
// standard.h
//
// Created 3/7/2016 7:31:18 AM by Tim
//
//****************************************************************************

#pragma once
#pragma pack(4)

#include <sam.h>
#include <sam_spec.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

// Standard data types
typedef	uint8_t			byte;
typedef	int8_t			sbyte;
typedef unsigned int	uint;
typedef unsigned short	ushort;
typedef unsigned long	ulong;

// Common macros
#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif
#define STRLEN(str)		(sizeof str - 1)	// characters in string literal
#define ADDOFFSET(p, o)	(((byte *)(p)) + (o))
#define NO_INLINE_ATTR	__attribute__((noinline))
#define INLINE_ATTR		__attribute__((always_inline))
#define DEPRECATED_ATTR	__attribute__((deprecated))
#define PACKED_ATTR		__attribute__((packed))
#define ALIGNED_ATTR(typ)	__attribute__((aligned(sizeof(typ))))
#define RAMFUNC_ATTR	__attribute__ ((section(".ramfunc")))
#define NAKED_ATTR		__attribute__ ((naked))
#define VERSION_INFO	__attribute__ ((section(".version_info")))
#define LOBYTE(w)       ((byte)(w))
#define HIBYTE(w)       ((byte)((ushort)(w) >> 8))
#define DIV_UINT_RND(x, y)	(((x) + (y) / 2) / (y))	// use this macro in constants
#define CONCAT_(x,y)	x##y
#define CONCAT(x,y)		CONCAT_(x,y)
#define CAT3_(x,y,z)	x##y##z
#define CAT3(x,y,z)		CAT3_(x,y,z)
#define STRINGIFY_(x)	#x
#define STRINGIFY(x)	STRINGIFY_(x)
#define LOG2(x)			(31 - __builtin_clz(x))
#define NOP				asm volatile ("nop\n")

inline bool CompSign(int s1, int s2)		{ return (s1 ^ s2) >= 0; }

// Rounded integer division/shifting
inline int ShiftIntRnd(int n, int s)		{ return ((n >> (s - 1)) + 1) >> 1; }
inline uint ShiftUintRnd(uint n, int s)		{ return ((n >> (s - 1)) + 1) >> 1; }
inline uint DivUintRnd(uint n, uint d)		{ return DIV_UINT_RND(n, d); }
inline int DivIntByUintRnd(int n, uint d)
{ 
	int sgn = n >> (sizeof(n)*8-1);	// 0 or -1
	return (n + (int)(((d / 2) ^ sgn) - sgn)) / (int)d; 
}
inline int DivIntRnd(int n, int d)		
{ 
	int rnd = d / 2;
	return (n + ((n ^ d) < 0 ? -rnd : rnd)) / d; 
}

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

typedef union
{
	ulong	ul;
	long	l;
	float	flt;
	byte	arb[4];
	ushort	arus[2];
	struct
	{
		ushort	uLo16;
		ushort	uHi16;
	};
	struct
	{
		byte	bLo;
		byte	bMidLo;
		byte	bMidHi;
		byte	bHi;
	};
} LONG_BYTES;

//****************************************************************************
// Include I/O help for this MCU

#if defined(__SAM_C_D__)
	#include <SamC-D/IoHelp.h>
#elif defined(__SAM_3_4__)
	#include <Sam3-4/IoHelp.h>
#endif
