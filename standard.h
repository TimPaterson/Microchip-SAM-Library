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
#define ADDOFFSET(p, o)	(((BYTE *)(p)) + (o))
#define NO_INLINE_ATTR	__attribute__((noinline))
#define INLINE_ATTR		__attribute__((always_inline))
#define DEPRECATED_ATTR	__attribute__((deprecated))
#define PACKED_ATTR		__attribute__((packed))
#define ALIGNED_ATTR(a)	__attribute__((aligned(a)))
#define _BV(bit)		(1 << (bit))
#define LOBYTE(w)       ((BYTE)(ushort)(w))
#define HIBYTE(w)       ((BYTE)((ushort)(w) >> 8))
#define DIV_INT_ROUND(x, y)	(((x) + (y) / 2) / (y))
#define CONCAT_(x,y)	x##y
#define CONCAT(x,y)		CONCAT_(x,y)
#define CAT3_(x,y,z)	x##y##z
#define CAT3(x,y,z)		CAT3_(x,y,z)

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
		BYTE	bLo;
		BYTE	bMidLo;
		BYTE	bMidHi;
		BYTE	bHi;
	};
} LONG_BYTES;

#ifdef __cplusplus

//*********************************************************************
// Bit I/O helpers

// For PORTA and PORTB explicitly
inline void SetPinsA(uint pins)		{ PORT_IOBUS->Group[0].OUTSET.reg = pins; }
inline void SetPinsB(uint pins)		{ PORT_IOBUS->Group[1].OUTSET.reg = pins; }
inline void ClearPinsA(uint pins)	{ PORT_IOBUS->Group[0].OUTCLR.reg = pins; }
inline void ClearPinsB(uint pins)	{ PORT_IOBUS->Group[1].OUTCLR.reg = pins; }
inline void TogglePinsA(uint pins)	{ PORT_IOBUS->Group[0].OUTTGL.reg = pins; }
inline void TogglePinsB(uint pins)	{ PORT_IOBUS->Group[1].OUTTGL.reg = pins; }

// For any port using port number (0 = PORTA, etc.)
inline void SetPins(uint pins, int iPort)		{ PORT_IOBUS->Group[iPort].OUTSET.reg = pins; }
inline void ClearPins(uint pins, int iPort)		{ PORT_IOBUS->Group[iPort].OUTCLR.reg = pins; }
inline void TogglePins(uint pins, int iPort)	{ PORT_IOBUS->Group[iPort].OUTTGL.reg = pins; }

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