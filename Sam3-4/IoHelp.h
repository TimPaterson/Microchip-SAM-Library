//****************************************************************************
// IoHelp.h for SAM 3 and SAM 4 devices
//
// Created 3/7/2016 7:31:18 AM by Tim
//
//****************************************************************************

#pragma once


inline void wdt_reset() {WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;}

#ifdef __cplusplus

//*********************************************************************
// Bit I/O helpers

static constexpr ulong ALL_PORT_PINS = 0xFFFFFFFF;

// Output
inline void SetPinsA(uint pins)			{ PIOA->PIO_SODR = pins; }
inline void SetPinsB(uint pins)			{ PIOB->PIO_SODR = pins; }
inline void ClearPinsA(uint pins)		{ PIOA->PIO_CODR = pins; }
inline void ClearPinsB(uint pins)		{ PIOB->PIO_CODR = pins; }
inline void WritePinsA(uint pins)		{ PIOA->PIO_ODSR = pins; }
inline void WritePinsB(uint pins)		{ PIOB->PIO_ODSR = pins; }
inline void SetOutMaskA(uint pins)		{ PIOA->PIO_OWER = pins; }
inline void SetOutMaskB(uint pins)		{ PIOB->PIO_OWER = pins; }
inline void ClearOutMaskA(uint pins)	{ PIOA->PIO_OWDR = pins; }
inline void ClearOutMaskB(uint pins)	{ PIOB->PIO_OWDR = pins; }
inline uint GetOutPinsA()				{ return PIOA->PIO_ODSR; }
inline uint GetOutPinsB()				{ return PIOB->PIO_ODSR; }

// Direction
inline void DirOutPinsA(uint pins)		{ PIOA->PIO_OER = pins; }
inline void DirOutPinsB(uint pins)		{ PIOB->PIO_OER = pins; }
inline void DirInPinsA(uint pins)		{ PIOA->PIO_ODR = pins; }
inline void DirInPinsB(uint pins)		{ PIOB->PIO_ODR = pins; }
inline uint GetDirPinsA()				{ return PIOA->PIO_OSR; }
inline uint GetDirPinsB()				{ return PIOB->PIO_OSR; }

// Input
inline uint GetPinsA()					{ return PIOA->PIO_PDSR; }
inline uint GetPinsB()					{ return PIOB->PIO_PDSR; }

#ifdef PIOC
// Output
inline void SetPinsC(uint pins)			{ PIOC->PIO_SODR = pins; }
inline void ClearPinsC(uint pins)		{ PIOC->PIO_CODR = pins; }
inline void WritePinsC(uint pins)		{ PIOC->PIO_ODSR = pins; }
inline void SetOutMaskC(uint pins)		{ PIOC->PIO_OWER = pins; }
inline void ClearOutMaskC(uint pins)	{ PIOC->PIO_OWDR = pins; }
inline uint GetOutPinsC()				{ return PIOC->PIO_ODSR; }

// Direction
inline void DirOutPinsC(uint pins)		{ PIOC->PIO_OER = pins; }
inline void DirInPinsC(uint pins)		{ PIOC->PIO_ODR = pins; }
inline uint GetDirPinsC()				{ return PIOC->PIO_OSR; }

// Input
inline uint GetPinsC()					{ return PIOC->PIO_PDSR; }

#endif	// ifdef PIOC

#endif	// ifdef __cplusplus
