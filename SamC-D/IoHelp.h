//****************************************************************************
// IoHelp.h for SAM C and SAM D devices
//
// Created 3/7/2016 7:31:18 AM by Tim
//
//****************************************************************************

#pragma once

#ifdef WDT_STATUS_SYNCBUSY
inline void wdt_reset() {if (!WDT->STATUS.bit.SYNCBUSY) WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;}
#elif defined(WDT_SYNCBUSY_CLEAR)
inline void wdt_reset() {if (!WDT->SYNCBUSY.bit.CLEAR) WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;}
#else
#error WDT reset not defined
#endif


#ifdef __cplusplus

//*********************************************************************
// Bit I/O helpers

static constexpr ulong ALL_PORT_PINS = 0xFFFFFFFF;

#define PORT_IOBUS_A	(&PORT_IOBUS->Group[0])
#define PORT_IOBUS_B	(&PORT_IOBUS->Group[1])

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
inline uint GetPins(int iPort)					{ return PORT_IOBUS->Group[iPort].IN.reg; }
inline uint GetPins(uint pins, int iPort)		{ return PORT_IOBUS->Group[iPort].IN.reg & pins; }

// For PORTA and PORTB explicitly
// Output
inline void SetPinsA(uint pins)			{ SetPins(pins, 0); }
inline void SetPinsB(uint pins)			{ SetPins(pins, 1); }
inline void ClearPinsA(uint pins)		{ ClearPins(pins, 0); }
inline void ClearPinsB(uint pins)		{ ClearPins(pins, 1); }
inline void TogglePinsA(uint pins)		{ TogglePins(pins, 0); }
inline void TogglePinsB(uint pins)		{ TogglePins(pins, 1); }
inline void WritePinsA(uint pins)		{ WritePins(pins, 0); }
inline void WritePinsB(uint pins)		{ WritePins(pins, 1); }
inline uint GetOutPinsA()				{ return GetOutPins(0); }
inline uint GetOutPinsB()				{ return GetOutPins(1); }

// Access as type LONG_BYTES above for individual byte access
#define PortSetA		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[0].OUTSET.reg)
#define PortSetB		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[1].OUTSET.reg)
#define PortClearA		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[0].OUTCLR.reg)
#define PortClearB		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[1].OUTCLR.reg)
#define PortToggleA		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[0].OUTTGL.reg)
#define PortToggleB		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[1].OUTTGL.reg)
#define PortWriteA		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[0].OUT.reg)
#define PortWriteB		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[1].OUT.reg)

// Direction
inline void DirOutPinsA(uint pins)		{ DirOutPins(pins, 0); }
inline void DirOutPinsB(uint pins)		{ DirOutPins(pins, 1); }
inline void DirInPinsA(uint pins)		{ DirInPins(pins, 0); }
inline void DirInPinsB(uint pins)		{ DirInPins(pins, 1); }
inline void DirTglPinsA(uint pins)		{ DirTglPins(pins, 0); }
inline void DirTglPinsB(uint pins)		{ DirTglPins(pins, 1); }
inline void DirWritePinsA(uint pins)	{ DirWritePins(pins, 0); }
inline void DirWritePinsB(uint pins)	{ DirWritePins(pins, 1); }
inline uint GetDirPinsA()				{ return GetDirPins(0); }
inline uint GetDirPinsB()				{ return GetDirPins(1); }

// Access as type LONG_BYTES above for individual byte access
#define PortDirOutA		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[0].DIRSET.reg)
#define PortDirOutB		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[1].DIRSET.reg)
#define PortDirInA		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[0].DIRCLR.reg)
#define PortDirInB		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[1].DIRCLR.reg)
#define PortDirTglA		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[0].DIRTGL.reg)
#define PortDirTglB		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[1].DIRTGL.reg)
#define PortDirWriteA	(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[0].DIR.reg)
#define PortDirWriteB	(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[1].DIR.reg)

// Input
inline uint GetPinsA(uint pins)			{ return GetPins(pins, 0); }
inline uint GetPinsB(uint pins)			{ return GetPins(pins, 1); }
inline uint GetPinsA()					{ return GetPins(0); }
inline uint GetPinsB()					{ return GetPins(1); }

// Access as type LONG_BYTES above for individual byte access
#define PortInA		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[0].IN.reg)
#define PortInB		(*(volatile LONG_BYTES *)&PORT_IOBUS->Group[1].IN.reg)

// Helper to use 8-bit access

inline void WriteByteOfReg32(volatile void *pv, uint val, ulong mask = 0)
{
	int		pos;

	mask |= val;
	pos = __builtin_ctzl(mask) / 8;
	if (__builtin_clzl(mask) / 8 == 3 - pos)
		((volatile byte *)pv)[pos] = val >> pos * 8;
	else
		*(volatile ulong *)pv = val;
}

//*********************************************************************
// Helpers to set up port configuration

inline void SetPortConfig(uint uConfig, uint uPins, int iPort)
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

inline void SetPortMuxConfig(uint uMux, uint uConfig, uint uPins, int iPort)
{
	SetPortConfig(		
		uConfig | 
		PORT_WRCONFIG_WRPMUX |
		PORT_WRCONFIG_PMUX(uMux) |
		PORT_WRCONFIG_PMUXEN,
		uPins,
		iPort
	);
}

inline void SetPortMux(uint uMux, uint uPins, int iPort)
{
	SetPortMuxConfig(uMux, 0, uPins, iPort);	
}

inline void SetPortMuxConfigA(uint uMux, uint uConfig, uint uPins)	{ SetPortMuxConfig(uMux, uConfig, uPins, 0); }
inline void SetPortMuxConfigB(uint uMux, uint uConfig, uint uPins)	{ SetPortMuxConfig(uMux, uConfig, uPins, 1); }
inline void SetPortMuxA(uint uMux, uint uPins)						{ SetPortMux(uMux, uPins, 0); }
inline void SetPortMuxB(uint uMux, uint uPins)						{ SetPortMux(uMux, uPins, 1); }

inline void SetPortMuxPin(uint uMux, uint uPin)
{
	SetPortConfig(		
		PORT_WRCONFIG_WRPMUX |
		PORT_WRCONFIG_PMUX(uMux) |
		PORT_WRCONFIG_PMUXEN,
		1 << (uPin & 0x1F),
		uPin >> 5
	);
}

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

inline void SetPinConfig(uint config, uint pin, int port)
{
	PORT->Group[port].PINCFG[pin].reg = config;
}

inline void SetPinConfigA(uint config, uint pin)
{
	SetPinConfig(config, pin, 0);
}

inline void SetPinConfigB(uint config, uint pin)
{
	SetPinConfig(config, pin, 1);
}

#endif	// ifdef __cplusplus
