//****************************************************************************
// standard.h
//
// Created 3/7/2016 7:31:18 AM by Tim
//
//****************************************************************************

#pragma once

#include <sam.h>
#include <sam_spec.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Standard data types
typedef	uint8_t			byte;
typedef	int8_t			sbyte;
typedef unsigned int	uint;
typedef unsigned short	ushort;
typedef unsigned long	ulong;
typedef	uint8_t			BYTE;
typedef	int8_t			SBYTE;
typedef wchar_t			WCHAR;
#ifndef __cplusplus
typedef	uint8_t			bool;
#endif

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
#define RAMFUNC			__attribute__ ((section(".ramfunc")))
#define VERSION_INFO	__attribute__ ((section(".version_info")))
#define _BV(bit)		(1 << (bit))
#define LOBYTE(w)       ((byte)(w))
#define HIBYTE(w)       ((byte)((ushort)(w) >> 8))
#define DIV_INT_ROUND(x, y)	(((x) + (y) / 2) / (y))	// deprecated name: doesn't work if < 0
#define DIV_UINT_RND(x, y)	(((x) + (y) / 2) / (y))	// use this macro in constants
#define CONCAT_(x,y)	x##y
#define CONCAT(x,y)		CONCAT_(x,y)
#define CAT3_(x,y,z)	x##y##z
#define CAT3(x,y,z)		CAT3_(x,y,z)
#define STRINGIFY_(x)	#x
#define STRINGIFY(x)	STRINGIFY_(x)

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

#ifdef WDT_STATUS_SYNCBUSY
inline void wdt_reset() {if (!WDT->STATUS.bit.SYNCBUSY) WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;}
#else
inline void wdt_reset() {if (!WDT->SYNCBUSY.bit.CLEAR) WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;}
#endif
static inline void cli()		{ __disable_irq(); }
static inline void sei()		{ __enable_irq(); }

typedef union
{
	ulong	ul;
	long	l;
	float	flt;
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

#ifdef __cplusplus

//*********************************************************************
// Bit I/O helpers

static constexpr ulong ALL_PORT_PINS = 0xFFFFFFFF;

// For PORTA and PORTB explicitly
// Output
inline void SetPinsA(uint pins)			{ PORT_IOBUS->Group[0].OUTSET.reg = pins; }
inline void SetPinsB(uint pins)			{ PORT_IOBUS->Group[1].OUTSET.reg = pins; }
inline void ClearPinsA(uint pins)		{ PORT_IOBUS->Group[0].OUTCLR.reg = pins; }
inline void ClearPinsB(uint pins)		{ PORT_IOBUS->Group[1].OUTCLR.reg = pins; }
inline void TogglePinsA(uint pins)		{ PORT_IOBUS->Group[0].OUTTGL.reg = pins; }
inline void TogglePinsB(uint pins)		{ PORT_IOBUS->Group[1].OUTTGL.reg = pins; }
inline void WritePinsA(uint pins)		{ PORT_IOBUS->Group[0].OUT.reg = pins; }
inline void WritePinsB(uint pins)		{ PORT_IOBUS->Group[1].OUT.reg = pins; }
inline uint GetOutPinsA()				{ return PORT_IOBUS->Group[0].OUT.reg; }
inline uint GetOutPinsB()				{ return PORT_IOBUS->Group[1].OUT.reg; }

// Access as type LONG_BYTES above for individual byte access
#define PortSetA		(*(LONG_BYTES *)&PORT_IOBUS->Group[0].OUTSET.reg)
#define PortSetB		(*(LONG_BYTES *)&PORT_IOBUS->Group[1].OUTSET.reg)
#define PortClearA		(*(LONG_BYTES *)&PORT_IOBUS->Group[0].OUTCLR.reg)
#define PortClearB		(*(LONG_BYTES *)&PORT_IOBUS->Group[1].OUTCLR.reg)
#define PortToggleA		(*(LONG_BYTES *)&PORT_IOBUS->Group[0].OUTTGL.reg)
#define PortToggleB		(*(LONG_BYTES *)&PORT_IOBUS->Group[1].OUTTGL.reg)
#define PortWriteA		(*(LONG_BYTES *)&PORT_IOBUS->Group[0].OUT.reg)
#define PortWriteB		(*(LONG_BYTES *)&PORT_IOBUS->Group[1].OUT.reg)

// Direction
inline void DirOutPinsA(uint pins)		{ PORT_IOBUS->Group[0].DIRSET.reg = pins; }
inline void DirOutPinsB(uint pins)		{ PORT_IOBUS->Group[1].DIRSET.reg = pins; }
inline void DirInPinsA(uint pins)		{ PORT_IOBUS->Group[0].DIRCLR.reg = pins; }
inline void DirInPinsB(uint pins)		{ PORT_IOBUS->Group[1].DIRCLR.reg = pins; }
inline void DirTglPinsA(uint pins)		{ PORT_IOBUS->Group[0].DIRTGL.reg = pins; }
inline void DirTglPinsB(uint pins)		{ PORT_IOBUS->Group[1].DIRTGL.reg = pins; }
inline void DirWritePinsA(uint pins)	{ PORT_IOBUS->Group[0].DIR.reg = pins; }
inline void DirWritePinsB(uint pins)	{ PORT_IOBUS->Group[1].DIR.reg = pins; }
inline uint GetDirPinsA()				{ return PORT_IOBUS->Group[0].DIR.reg; }
inline uint GetDirPinsB()				{ return PORT_IOBUS->Group[1].DIR.reg; }

// Access as type LONG_BYTES above for individual byte access
#define PortDirOutA		(*(LONG_BYTES *)&PORT_IOBUS->Group[0].DIRSET.reg)
#define PortDirOutB		(*(LONG_BYTES *)&PORT_IOBUS->Group[1].DIRSET.reg)
#define PortDirInA		(*(LONG_BYTES *)&PORT_IOBUS->Group[0].DIRCLR.reg)
#define PortDirInB		(*(LONG_BYTES *)&PORT_IOBUS->Group[1].DIRCLR.reg)
#define PortDirTglA		(*(LONG_BYTES *)&PORT_IOBUS->Group[0].DIRTGL.reg)
#define PortDirTglB		(*(LONG_BYTES *)&PORT_IOBUS->Group[1].DIRTGL.reg)
#define PortDirWriteA	(*(LONG_BYTES *)&PORT_IOBUS->Group[0].DIR.reg)
#define PortDirWriteB	(*(LONG_BYTES *)&PORT_IOBUS->Group[1].DIR.reg)

// Input
inline uint GetPinsA(uint pins)			{ return PORT_IOBUS->Group[0].IN.reg & pins; }
inline uint GetPinsB(uint pins)			{ return PORT_IOBUS->Group[1].IN.reg & pins; }
inline uint GetPinsA()					{ return PORT_IOBUS->Group[0].IN.reg; }
inline uint GetPinsB()					{ return PORT_IOBUS->Group[1].IN.reg; }

// Access as type LONG_BYTES above for individual byte access
#define PortInA		(*(LONG_BYTES *)&PORT_IOBUS->Group[0].IN.reg)
#define PortInB		(*(LONG_BYTES *)&PORT_IOBUS->Group[1].IN.reg)

// For any port using port number (0 = PORTA, etc.)
// Output
inline void SetPins(uint pins, int iPort)		{ PORT_IOBUS->Group[iPort].OUTSET.reg = pins; }
inline void ClearPins(uint pins, int iPort)		{ PORT_IOBUS->Group[iPort].OUTCLR.reg = pins; }
inline void TogglePins(uint pins, int iPort)	{ PORT_IOBUS->Group[iPort].OUTTGL.reg = pins; }
inline void WritePins(uint pins, int iPort)		{ PORT_IOBUS->Group[iPort].OUT.reg = pins; }
inline uint GetOutPins(int iPort)				{ return PORT_IOBUS->Group[iPort].OUT.reg; }

// Direction
inline void DirOutPins(uint pins, int iPort)	{ PORT_IOBUS->Group[iPort].DIRSET.reg = pins; }
inline void DirInPins(uint pins, int iPort)		{ PORT_IOBUS->Group[iPort].DIRCLR.reg = pins; }
inline void DirTglPins(uint pins, int iPort)	{ PORT_IOBUS->Group[iPort].DIRTGL.reg = pins; }
inline void DirWritePins(uint pins, int iPort)	{ PORT_IOBUS->Group[iPort].DIR.reg = pins; }
inline uint GetDirPins(int iPort)				{ return PORT_IOBUS->Group[iPort].DIR.reg; }

// Input
inline uint GetPins(uint pins, int iPort)		{ return PORT_IOBUS->Group[iPort].IN.reg & pins; }

// For PORTA and/or PORTB using 64-bit mask
inline void SetPins(uint64_t pins)
{
	if (pins & 0xFFFFFFFF)
		SetPinsA(pins & 0xFFFFFFFF);
		
	if (pins > 0xFFFFFFFF)
		SetPinsB(pins >> 32);
}

inline void ClearPins(uint64_t pins)
{
	if (pins & 0xFFFFFFFF)
		ClearPinsA(pins & 0xFFFFFFFF);
		
	if (pins > 0xFFFFFFFF)
		ClearPinsB(pins >> 32);
}

inline void TogglePins(uint64_t pins)
{
	if (pins & 0xFFFFFFFF)
		TogglePinsA(pins & 0xFFFFFFFF);
		
	if (pins > 0xFFFFFFFF)
		TogglePinsB(pins >> 32);
}

inline uint64_t GetPins(uint64_t pins)
{
	uint64_t	res = 0;

	if (pins & 0xFFFFFFFF)
		res = GetPinsA(pins & 0xFFFFFFFF);
		
	if (pins > 0xFFFFFFFF)
		res |= GetPinsB(pins >> 32);

	return res;
}

//*********************************************************************
// Helpers to set up port configuration

inline void SetPortConfig(uint uConfig, uint uPins, int iPort = 0)
{
	if (uPins & 0xFFFF)
	{
		// Set pins in lo 16 bits
		PORT->Group[iPort].WRCONFIG.reg =
			uConfig | 
			PORT_WRCONFIG_WRPINCFG | 
			PORT_WRCONFIG_PINMASK(uPins & 0xFFFF);
	}
	
	if (uPins > 0xFFFF)
	{
		// Set pins in hi 16 bits
		PORT->Group[iPort].WRCONFIG.reg =
			uConfig |
			PORT_WRCONFIG_WRPINCFG | 
			PORT_WRCONFIG_PINMASK(uPins >> 16) |
			PORT_WRCONFIG_HWSEL;
	}
}

inline void SetPortConfigA(uint uConfig, uint uPins) { SetPortConfig(uConfig, uPins, 0); }
inline void SetPortConfigB(uint uConfig, uint uPins) { SetPortConfig(uConfig, uPins, 1); }

inline void SetPortMux(uint uMux, uint uPins, int iPort = 0)
{
	SetPortConfig(		
		PORT_WRCONFIG_WRPMUX |
		PORT_WRCONFIG_PMUX(uMux) |
		PORT_WRCONFIG_INEN |
		PORT_WRCONFIG_PMUXEN,
		uPins,
		iPort
	);
}

inline void SetPortMuxA(uint uMux, uint uPins)	{ SetPortMux(uMux, uPins, 0); }
inline void SetPortMuxB(uint uMux, uint uPins)	{ SetPortMux(uMux, uPins, 1); }

inline void SetPortMuxPin(uint uMux, uint uPin)
{
	SetPortConfig(		
		PORT_WRCONFIG_WRPMUX |
		PORT_WRCONFIG_PMUX(uMux) |
		PORT_WRCONFIG_INEN |
		PORT_WRCONFIG_PMUXEN,
		1 << (uPin & 0x1F),
		uPin >> 5
	);
}
#endif

enum PORT_MUX
{
	PORT_MUX_A,
	PORT_MUX_B,
	PORT_MUX_C,
	PORT_MUX_D,
	PORT_MUX_E,
	PORT_MUX_F,
	PORT_MUX_G,
	PORT_MUX_H,
	PORT_MUX_I
};